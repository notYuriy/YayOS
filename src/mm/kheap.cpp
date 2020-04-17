#include <mm/kheap.hpp>
#include <mm/kvmmngr.hpp>

namespace memory {

#pragma pack(1)
    struct ObjectHeader {
        Uint64 realSize;
        ObjectHeader* next;
        char data[];
        INLINE void* getData() { return (void*)data; }
        INLINE static ObjectHeader* getHeader(void* ptr) {
            return (ObjectHeader*)(ptr)-1;
        }
    };
#pragma pack(0)

    static_assert(sizeof(ObjectHeader) == 16);

#pragma pack(1)
    struct SmallObjectPool {
#pragma pack(1)
        struct PoolMetadata {
            ObjectHeader* first;
            Uint64 objectSize;
            SmallObjectPool* prev;
            SmallObjectPool* next;
            Uint64 objectsCount;
            Uint64 arenaId;
        } meta;
#pragma pack(0)

        static_assert(sizeof(PoolMetadata) == 48);

        INLINE ObjectHeader* headerAt(Uint64 index) const {
            return (ObjectHeader*)(((Uint64)this) + sizeof(PoolMetadata) +
                                   (meta.objectSize + sizeof(ObjectHeader)) *
                                       index);
        }
        INLINE Uint64 maxCount() const {
            return (4096 - 16) / (meta.objectSize + sizeof(PoolMetadata));
        }
        INLINE static Uint64 maxCount(Uint64 objectSize) {
            return (4096 - 16) / (objectSize + sizeof(PoolMetadata));
        }
        INLINE void init(Uint64 objSize) {
            meta.first = headerAt(0);
            meta.objectSize = objSize;
            meta.objectsCount = maxCount();
            for (Uint64 i = 0; i < meta.objectsCount - 1; ++i) {
                headerAt(i)->next = headerAt(i + 1);
            }
            headerAt(meta.objectsCount - 1)->next = nullptr;
        }
        INLINE ObjectHeader* alloc() {
            if (meta.first == nullptr) {
                return nullptr;
            }
            ObjectHeader* result = meta.first;
            meta.first = meta.first->next;
            meta.objectsCount--;
            return result;
        }
        INLINE void free(ObjectHeader* header) {
            header->next = meta.first;
            meta.first = header;
            meta.objectsCount++;
        }
    };
#pragma pack(0)

    void KernelHeapArena::cutPoolFrom(SmallObjectPool* pool, Uint64 sizeIndex,
                                      Uint64 headsIndex) {
        if (pool->meta.prev == nullptr) {
            poolHeadsArray[sizeIndex][headsIndex] = pool->meta.next;
        } else {
            pool->meta.prev->meta.next = pool->meta.next;
        }
        if (pool->meta.next != nullptr) {
            pool->meta.next->meta.prev = pool->meta.prev;
        }
    }

    void KernelHeapArena::insertPoolTo(SmallObjectPool* pool, Uint64 sizeIndex,
                                       Uint64 headsIndex) {
        pool->meta.next = poolHeadsArray[sizeIndex][headsIndex];
        pool->meta.prev = nullptr;
        if (pool->meta.next != nullptr) {
            pool->meta.next->meta.prev = pool;
        }
        poolHeadsArray[sizeIndex][headsIndex] = pool;
    }

    bool KernelHeapArena::isSlubEmpty(Uint64 sizeIndex) {
        return poolsLastCheckedIndices[sizeIndex] == (poolsMaxCount[sizeIndex]);
    }

    ObjectHeader* KernelHeapArena::allocFromSlubs(Uint64 size) {
        Uint64 index = size / 16;
        if (poolsLastCheckedIndices[index] == 0) {
            panic("[KernelHeap] Assertion failed: last checked index is zero");
        }
        for (; poolsLastCheckedIndices[index] < poolsMaxCount[index] + 1;
             ++poolsLastCheckedIndices[index]) {
            Uint64 indexInPoolHeads = poolsLastCheckedIndices[index];
            if (poolHeadsArray[index][indexInPoolHeads] != nullptr) {
                SmallObjectPool* pool = poolHeadsArray[index][indexInPoolHeads];
                cutPoolFrom(pool, index, pool->meta.objectsCount);
                ObjectHeader* allocResult = pool->alloc();
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

    bool KernelHeapArena::getNewPool(Uint64 size) {
        VAddr page =
            KernelVirtualAllocator::getMapping(4096, 0, defaultKernelFlags);
        if (page == 0) {
            return false;
        }
        SmallObjectPool* pool = (SmallObjectPool*)page;
        //pool->meta.arenaId = arenaId;
        pool->init(size);
        insertPoolTo(pool, size / 16, poolsMaxCount[size / 16]);
        poolsLastCheckedIndices[size / 16] = poolsMaxCount[size / 16];
        return true;
    }

    void* KernelHeapArena::alloc(Uint64 size) {
        lock.lock();
        ObjectHeader* result = allocFromSlubs(size);
        if (result == nullptr) {
            if (!getNewPool(size)) {
                lock.unlock();
                return nullptr;
            }
            poolsLastCheckedIndices[size / 16] = poolsMaxCount[size / 16];
            result = allocFromSlubs(size);
        }
        lock.unlock();
        return result->getData();
    }

    void KernelHeapArena::free(void* loc) {
        lock.lock();
        ObjectHeader* header = ObjectHeader::getHeader(loc);
        Uint64 poolAddr = alignDown((Uint64)header, 4096);
        SmallObjectPool* pool = (SmallObjectPool*)poolAddr;
        if (poolsLastCheckedIndices[pool->meta.objectSize / 16] <
            pool->meta.objectsCount) {
            poolsLastCheckedIndices[pool->meta.objectSize] =
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

    void KernelHeapArena::init(Uint64 id) {
        arenaId = id;
        Uint64 initMemory;
        Uint64 requiredMem = 0;
        for (Uint64 i = 0; i < poolsSizesCount; ++i) {
            poolsMaxCount[i] = SmallObjectPool::maxCount(16 * i);
            poolsLastCheckedIndices[i] = poolsMaxCount[i];
            requiredMem += (poolsMaxCount[i] + 1) * sizeof(SmallObjectPool*);
        }
        Uint64 alignedRequiredMem = alignUp(requiredMem, 4096);
        initMemory = KernelVirtualAllocator::getMapping(alignedRequiredMem, 0,
                                                        defaultKernelFlags);
        for (Uint64 i = 0; i < poolsSizesCount; ++i) {
            poolHeadsArray[i] = (SmallObjectPool**)initMemory;
            initMemory += (SmallObjectPool::maxCount(16 * i) + 1) *
                          sizeof(SmallObjectPool*);
        }
        lock.lockValue = 0;
    }

    bool KernelHeap::initialized;
    KernelHeapArena* KernelHeap::arenas;

    void KernelHeap::init() {
        Uint64 areanasCount = getCoresCount();
        if (!KernelVirtualAllocator::isInitialized()) {
            panic("[Kernel Heap] Dependency \"KernelVirtualAllocator\" is not "
                  "satisfied\n\r");
        }
        memory::VAddr arenasAddr = KernelVirtualAllocator::getMapping(
            alignUp(sizeof(KernelHeapArena) * areanasCount, 4096), 0, defaultKernelFlags);
        arenas = (KernelHeapArena*)arenasAddr;
        for (Uint64 i = 0; i < areanasCount; ++i) {
            arenas[i].init(i);
        }
        initialized = true;
    }

    void* KernelHeap::alloc(Uint64 size) { 
        if (size == 0) {
            return nullptr;
        }
        size = alignUp(size, 16);
        if (size >= maxSizeForSlubs) {
            ObjectHeader* header =
                (ObjectHeader*)KernelVirtualAllocator::getMapping(
                    alignUp(size, 4096), 0, defaultKernelFlags);
            header->realSize = alignUp(size, 4096);
            return header->getData();
        }
        return arenas[getCoreHeapId()].alloc(size);
    }

    void KernelHeap::free(void* ptr) { 
        ObjectHeader* header = ObjectHeader::getHeader(ptr);
        if (header->realSize > maxSizeForSlubs) {
            KernelVirtualAllocator::unmapAt((VAddr)header, header->realSize);
            return;
        }
        Uint64 poolAddr = alignDown((Uint64)header, 4096);
        SmallObjectPool* pool = (SmallObjectPool*)poolAddr;
        arenas[pool->meta.arenaId].free(ptr);
    }

}; // namespace memory