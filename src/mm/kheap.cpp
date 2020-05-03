#include <mm/kheap.hpp>
#include <mm/kvmmngr.hpp>
#include <proc/mutex.hpp>

namespace memory {
    bool KernelHeap::initialized;

#pragma pack(1)
    struct ObjectHeader {
        uint64_t realSize;
        ObjectHeader *next;
        char data[];
        INLINE void *getData() { return (void *)data; }
        INLINE static ObjectHeader *getHeader(void *ptr) {
            return (ObjectHeader *)(ptr)-1;
        }
    };
#pragma pack(0)

    static_assert(sizeof(ObjectHeader) == 16);

#pragma pack(1)
    struct SmallObjectPool {
#pragma pack(1)
        struct PoolMetadata {
            ObjectHeader *first;
            uint64_t objectSize;
            SmallObjectPool *prev;
            SmallObjectPool *next;
            uint64_t objectsCount;
            uint64_t : 64;
        } meta;
#pragma pack(0)

        static_assert(sizeof(PoolMetadata) == 48);

        INLINE ObjectHeader *headerAt(uint64_t index) const {
            return (ObjectHeader *)(((uint64_t)this) + sizeof(PoolMetadata) +
                                    (meta.objectSize + sizeof(ObjectHeader)) *
                                        index);
        }
        INLINE uint64_t maxCount() const {
            return (4096 - 16) / (meta.objectSize + sizeof(PoolMetadata));
        }
        INLINE static uint64_t maxCount(uint64_t objectSize) {
            return (4096 - 16) / (objectSize + sizeof(PoolMetadata));
        }
        INLINE void init(uint64_t objSize) {
            meta.first = headerAt(0);
            meta.objectSize = objSize;
            meta.objectsCount = maxCount();
            for (uint64_t i = 0; i < meta.objectsCount - 1; ++i) {
                headerAt(i)->next = headerAt(i + 1);
            }
            headerAt(meta.objectsCount - 1)->next = nullptr;
        }
        INLINE ObjectHeader *alloc() {
            if (meta.first == nullptr) {
                return nullptr;
            }
            ObjectHeader *result = meta.first;
            meta.first = meta.first->next;
            meta.objectsCount--;
            return result;
        }
        INLINE void free(ObjectHeader *header) {
            header->next = meta.first;
            meta.first = header;
            meta.objectsCount++;
        }
    };
#pragma pack(0)

    const uint64_t maxSizeForSlubs = 2000;
    constexpr uint64_t poolsSizesCount = maxSizeForSlubs / 16;

    SmallObjectPool **poolHeadsArray[poolsSizesCount];
    uint64_t poolsMaxCount[poolsSizesCount];
    uint64_t poolsLastCheckedIndices[poolsSizesCount];
    proc::Mutex heapMutex;

    void cutPoolFrom(SmallObjectPool *pool, uint64_t sizeIndex,
                     uint64_t headsIndex) {
        if (pool->meta.prev == nullptr) {
            poolHeadsArray[sizeIndex][headsIndex] = pool->meta.next;
        } else {
            pool->meta.prev->meta.next = pool->meta.next;
        }
        if (pool->meta.next != nullptr) {
            pool->meta.next->meta.prev = pool->meta.prev;
        }
    }

    void insertPoolTo(SmallObjectPool *pool, uint64_t sizeIndex,
                      uint64_t headsIndex) {
        pool->meta.next = poolHeadsArray[sizeIndex][headsIndex];
        pool->meta.prev = nullptr;
        if (pool->meta.next != nullptr) {
            pool->meta.next->meta.prev = pool;
        }
        poolHeadsArray[sizeIndex][headsIndex] = pool;
    }

    INLINE bool isSlubEmpty(uint64_t sizeIndex) {
        return poolsLastCheckedIndices[sizeIndex] == (poolsMaxCount[sizeIndex]);
    }

    ObjectHeader *allocFromSlubs(uint64_t size) {
        uint64_t index = size / 16;
        if (poolsLastCheckedIndices[index] == 0) {
            panic("[KernelHeap] Assertion failed: last checked index is zero");
        }
        for (; poolsLastCheckedIndices[index] < poolsMaxCount[index] + 1;
             ++poolsLastCheckedIndices[index]) {
            uint64_t indexInPoolHeads = poolsLastCheckedIndices[index];
            if (poolHeadsArray[index][indexInPoolHeads] != nullptr) {
                SmallObjectPool *pool = poolHeadsArray[index][indexInPoolHeads];
                cutPoolFrom(pool, index, pool->meta.objectsCount);
                ObjectHeader *allocResult = pool->alloc();
                insertPoolTo(pool, index, pool->meta.objectsCount);
                --poolsLastCheckedIndices[index];
                if (poolsLastCheckedIndices[index] == 0) {
                    poolsLastCheckedIndices[index] = 1;
                }
                return allocResult;
            }
        }
        return nullptr;
    }

    bool getNewPool(uint64_t size) {
        vaddr_t page =
            KernelVirtualAllocator::getMapping(4096, 0, defaultKernelFlags);
        if (page == 0) {
            return false;
        }
        SmallObjectPool *pool = (SmallObjectPool *)page;
        pool->init(size);
        insertPoolTo(pool, size / 16, poolsMaxCount[size / 16]);
        poolsLastCheckedIndices[size / 16] = poolsMaxCount[size / 16];
        return true;
    }

    void *KernelHeap::alloc(uint64_t size) {
        if (size == 0) {
            return nullptr;
        }
        size = alignUp(size, 16);
        if (size >= maxSizeForSlubs) {
            ObjectHeader *header =
                (ObjectHeader *)KernelVirtualAllocator::getMapping(
                    alignUp(size, 4096), 0, defaultKernelFlags);
            header->realSize = alignUp(size, 4096);
            return header->getData();
        }
        heapMutex.lock();
        ObjectHeader *result = allocFromSlubs(size);
        if (result == nullptr) {
            if (!getNewPool(size)) {
                heapMutex.unlock();
                return nullptr;
            }
            poolsLastCheckedIndices[size / 16] = poolsMaxCount[size / 16];
            result = allocFromSlubs(size);
        }
        heapMutex.unlock();
        return result->getData();
    }

    void KernelHeap::free(void *loc) {
        ObjectHeader *header = ObjectHeader::getHeader(loc);
        if (header->realSize > maxSizeForSlubs) {
            KernelVirtualAllocator::unmapAt((vaddr_t)header, header->realSize);
            return;
        }
        heapMutex.lock();
        uint64_t poolAddr = alignDown((uint64_t)header, 4096);
        SmallObjectPool *pool = (SmallObjectPool *)poolAddr;
        if (poolsLastCheckedIndices[pool->meta.objectSize / 16] <
            pool->meta.objectsCount) {
            poolsLastCheckedIndices[pool->meta.objectSize] =
                pool->meta.objectsCount;
        }
        cutPoolFrom(pool, pool->meta.objectSize / 16, pool->meta.objectsCount);
        pool->free(header);
        if (pool->meta.objectsCount == pool->maxCount()) {
            KernelVirtualAllocator::unmapAt(poolAddr, 4096);
            heapMutex.unlock();
            return;
        }
        insertPoolTo(pool, pool->meta.objectSize / 16, pool->meta.objectsCount);
        heapMutex.unlock();
    }

    void KernelHeap::init() {
        if (!KernelVirtualAllocator::isInitialized()) {
            panic("[KernelHeap] Dependency KernelVirtualAllocator is not "
                  "satisfied\n\r");
        }
        uint64_t initMemory;
        uint64_t requiredMem = 0;
        for (uint64_t i = 0; i < poolsSizesCount; ++i) {
            poolsMaxCount[i] = SmallObjectPool::maxCount(16 * i);
            poolsLastCheckedIndices[i] = poolsMaxCount[i];
            requiredMem += (poolsMaxCount[i] + 1) * sizeof(SmallObjectPool *);
        }
        uint64_t alignedRequiredMem = alignUp(requiredMem, 4096);
        initMemory = KernelVirtualAllocator::getMapping(alignedRequiredMem, 0,
                                                        defaultKernelFlags);
        for (uint64_t i = 0; i < poolsSizesCount; ++i) {
            poolHeadsArray[i] = (SmallObjectPool **)initMemory;
            initMemory += (SmallObjectPool::maxCount(16 * i) + 1) *
                          sizeof(SmallObjectPool *);
        }
        heapMutex.init();
        initialized = true;
    }

}; // namespace memory