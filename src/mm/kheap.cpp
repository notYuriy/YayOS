#include <mm/kheap.hpp>
#include <mm/kvmmngr.hpp>

namespace memory {

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
            uint64_t arenaId;
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

    void KernelHeapArena::cutPoolFrom(SmallObjectPool *pool, uint64_t sizeIndex,
                                      uint64_t headsIndex) {
        if (pool->meta.prev == nullptr) {
            m_poolHeadsArray[sizeIndex][headsIndex] = pool->meta.next;
        } else {
            pool->meta.prev->meta.next = pool->meta.next;
        }
        if (pool->meta.next != nullptr) {
            pool->meta.next->meta.prev = pool->meta.prev;
        }
    }

    void KernelHeapArena::insertPoolTo(SmallObjectPool *pool,
                                       uint64_t sizeIndex,
                                       uint64_t headsIndex) {
        pool->meta.next = m_poolHeadsArray[sizeIndex][headsIndex];
        pool->meta.prev = nullptr;
        if (pool->meta.next != nullptr) {
            pool->meta.next->meta.prev = pool;
        }
        m_poolHeadsArray[sizeIndex][headsIndex] = pool;
    }

    bool KernelHeapArena::isSlubEmpty(uint64_t sizeIndex) {
        return m_poolsLastCheckedIndices[sizeIndex] ==
               (m_poolsMaxCount[sizeIndex]);
    }

    ObjectHeader *KernelHeapArena::allocFromSlubs(uint64_t size) {
        uint64_t index = size / 16;
        if (m_poolsLastCheckedIndices[index] == 0) {
            panic("[KernelHeap] Assertion failed: last checked index is zero");
        }
        for (; m_poolsLastCheckedIndices[index] < m_poolsMaxCount[index] + 1;
             ++m_poolsLastCheckedIndices[index]) {
            uint64_t indexInPoolHeads = m_poolsLastCheckedIndices[index];
            if (m_poolHeadsArray[index][indexInPoolHeads] != nullptr) {
                SmallObjectPool *pool =
                    m_poolHeadsArray[index][indexInPoolHeads];
                cutPoolFrom(pool, index, pool->meta.objectsCount);
                ObjectHeader *allocResult = pool->alloc();
                insertPoolTo(pool, index, pool->meta.objectsCount);
                --m_poolsLastCheckedIndices[index];
                if (m_poolsLastCheckedIndices[index] == 0) {
                    m_poolsLastCheckedIndices[index] = 1;
                }
                return allocResult;
            }
        }
        return nullptr;
    }

    bool KernelHeapArena::getNewPool(uint64_t size) {
        VAddr page =
            KernelVirtualAllocator::getMapping(4096, 0, defaultKernelFlags);
        if (page == 0) {
            return false;
        }
        SmallObjectPool *pool = (SmallObjectPool *)page;
        // pool->meta.arenaId = arenaId;
        pool->init(size);
        insertPoolTo(pool, size / 16, m_poolsMaxCount[size / 16]);
        m_poolsLastCheckedIndices[size / 16] = m_poolsMaxCount[size / 16];
        return true;
    }

    void *KernelHeapArena::alloc(uint64_t size) {
        lock.lock();
        ObjectHeader *result = allocFromSlubs(size);
        if (result == nullptr) {
            if (!getNewPool(size)) {
                lock.unlock();
                return nullptr;
            }
            m_poolsLastCheckedIndices[size / 16] = m_poolsMaxCount[size / 16];
            result = allocFromSlubs(size);
        }
        lock.unlock();
        return result->getData();
    }

    void KernelHeapArena::free(void *loc) {
        lock.lock();
        ObjectHeader *header = ObjectHeader::getHeader(loc);
        uint64_t poolAddr = alignDown((uint64_t)header, 4096);
        SmallObjectPool *pool = (SmallObjectPool *)poolAddr;
        if (m_poolsLastCheckedIndices[pool->meta.objectSize / 16] <
            pool->meta.objectsCount) {
            m_poolsLastCheckedIndices[pool->meta.objectSize] =
                pool->meta.objectsCount;
        }
        cutPoolFrom(pool, pool->meta.objectSize / 16, pool->meta.objectsCount);
        pool->free(header);
        if (pool->meta.objectsCount == pool->maxCount()) {
            KernelVirtualAllocator::unmapAt(poolAddr, 4096);
            lock.unlock();
            return;
        }
        insertPoolTo(pool, pool->meta.objectSize / 16, pool->meta.objectsCount);
        lock.unlock();
    }

    void KernelHeapArena::init(uint64_t id) {
        arenaId = id;
        uint64_t initMemory;
        uint64_t requiredMem = 0;
        for (uint64_t i = 0; i < poolsSizesCount; ++i) {
            m_poolsMaxCount[i] = SmallObjectPool::maxCount(16 * i);
            m_poolsLastCheckedIndices[i] = m_poolsMaxCount[i];
            requiredMem += (m_poolsMaxCount[i] + 1) * sizeof(SmallObjectPool *);
        }
        uint64_t alignedRequiredMem = alignUp(requiredMem, 4096);
        initMemory = KernelVirtualAllocator::getMapping(alignedRequiredMem, 0,
                                                        defaultKernelFlags);
        for (uint64_t i = 0; i < poolsSizesCount; ++i) {
            m_poolHeadsArray[i] = (SmallObjectPool **)initMemory;
            initMemory += (SmallObjectPool::maxCount(16 * i) + 1) *
                          sizeof(SmallObjectPool *);
        }
        lock.m_lockValue = 0;
    }

    bool KernelHeap::m_initialized;
    KernelHeapArena *KernelHeap::m_arenas;

    void KernelHeap::init() {
        uint64_t areanasCount = getCoresCount();
        if (!KernelVirtualAllocator::isInitialized()) {
            panic("[Kernel Heap] Dependency \"KernelVirtualAllocator\" is not "
                  "satisfied\n\r");
        }
        memory::VAddr arenasAddr = KernelVirtualAllocator::getMapping(
            alignUp(sizeof(KernelHeapArena) * areanasCount, 4096), 0,
            defaultKernelFlags);
        m_arenas = (KernelHeapArena *)arenasAddr;
        for (uint64_t i = 0; i < areanasCount; ++i) {
            m_arenas[i].init(i);
        }
        m_initialized = true;
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
        return m_arenas[getCoreHeapId()].alloc(size);
    }

    void KernelHeap::free(void *ptr) {
        ObjectHeader *header = ObjectHeader::getHeader(ptr);
        if (header->realSize > maxSizeForSlubs) {
            KernelVirtualAllocator::unmapAt((VAddr)header, header->realSize);
            return;
        }
        uint64_t poolAddr = alignDown((uint64_t)header, 4096);
        SmallObjectPool *pool = (SmallObjectPool *)poolAddr;
        m_arenas[pool->meta.arenaId].free(ptr);
    }

}; // namespace memory