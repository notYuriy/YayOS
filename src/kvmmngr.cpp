#include <kvmmngr.hpp>
#include <vmmap.hpp>

namespace memory {

    bool KernelVirtualAllocator::initialized = false;

    struct MemoryArea {
        MemoryArea* next;
        MemoryArea* prev;
        VAddr offset;
        Uint64 size;
    } PACKED;

    static_assert(sizeof(MemoryArea) == 32);

    struct MemoryAreaPool* poolHeads[128];
    volatile Uint64 lastCheckedIndex;
    MemoryArea* kernelAreas;

    struct MemoryAreaPool {
        MemoryArea* first;
        MemoryAreaPool* next;
        MemoryAreaPool* prev;
        Uint64 count;
        MemoryArea areas[127];

        void init() {
            count = 127;
            first = &areas[0];
            for (Uint64 i = 0; i < 126; ++i) {
                areas[i].next = &areas[i + 1];
            }
            areas[126].next = nullptr;
        }

        MemoryArea* allocNode() {
            MemoryArea* result = first;
            first = first->next;
            count--;
            return result;
        }

        void freeNode(MemoryArea* node) {
            node->next = first;
            first = node;
            count++;
        }
    };

    static_assert(sizeof(MemoryAreaPool) == 4096);

    void cutPool(MemoryAreaPool* pool) {
        if (pool->prev == nullptr) {
            poolHeads[pool->count] = pool->next;
        } else {
            pool->prev->next = pool->next;
        }
        if (pool->next != nullptr) {
            pool->next->prev = pool->prev;
        }
    }

    void insertPool(MemoryAreaPool* pool) {
        pool->prev = nullptr;
        pool->next = poolHeads[pool->count];
        if (pool->next != nullptr) {
            pool->next->prev = pool;
        } else {
            poolHeads[pool->count] = pool;
        }
    }

    MemoryArea* allocMemoryArea() {
        for (; lastCheckedIndex < 128ULL; ++lastCheckedIndex) {
            if (poolHeads[lastCheckedIndex] == nullptr) {
                continue;
            }
            MemoryAreaPool* pool = poolHeads[lastCheckedIndex];
            cutPool(pool);
            MemoryArea* result = pool->allocNode();
            insertPool(pool);
            if (lastCheckedIndex != 1) {
                lastCheckedIndex--;
            }
            result->next = nullptr;
            result->prev = nullptr;
            return result;
        }
        return nullptr;
    }

    void freeMemoryArea(MemoryArea* node) {
        MemoryAreaPool* pool = (MemoryAreaPool*)alignDown((Uint64)node, 4096);
        cutPool(pool);
        pool->freeNode(node);
        insertPool(pool);
        if (pool->count < lastCheckedIndex) {
            lastCheckedIndex = pool->count;
        }
        return;
    }

    INLINE void insertBefore(MemoryArea* area, MemoryArea* point) {
        MemoryArea* prev = point->prev;
        if (prev != nullptr) {
            prev->next = area;
        } else {
            kernelAreas = area;
        }
        area->prev = prev;
        area->next = point;
        point->prev = area;
    }

    INLINE void cutNode(MemoryArea* area) {
        if (area->prev != nullptr) {
            area->prev->next = area->next;
        }
        if (area->next != nullptr) {
            area->next->prev = area->prev;
        }
    }

    INLINE void mergeAdjacent(MemoryArea* area) {
        bool mergeLeft = false, mergeRight = false;
        if (area->prev != nullptr) {
            mergeLeft = (area->prev->offset + area->prev->size == area->offset);
        }
        if (area->next != nullptr) {
            mergeRight = (area->offset + area->size == area->next->offset);
        }
        if (mergeLeft) {
            MemoryArea* prev = area->prev;
            if (area->prev->prev == nullptr) {
                kernelAreas = area;
            }
            area->offset = area->prev->offset;
            area->size += area->prev->size;
            cutNode(prev);
            freeMemoryArea(prev);
        }
        if (mergeRight) {
            MemoryArea* next = area->next;
            area->size += area->next->size;
            cutNode(next);
            freeMemoryArea(next);
        }
    }

    void freeMapping(MemoryArea* area) {
        MemoryArea* current = kernelAreas;
        if (current == nullptr) {
            kernelAreas = area;
            return;
        }
        while (current->next != nullptr) {
            if (current->offset > area->offset) {
                insertBefore(area, current);
            }
            current = current->next;
        }
        if (current->offset > area->offset) {
            insertBefore(area, current);
        } else {
            current->next = area;
            area->prev = current;
        }
        mergeAdjacent(area);
    }

    void freePools() {
        while (true) {
            if (poolHeads[127] == nullptr) {
                return;
            }
            if (lastCheckedIndex == 127 && poolHeads[127]->next == nullptr) {
                return;
            }
            MemoryArea* area = allocMemoryArea();
            MemoryAreaPool* pool = poolHeads[127];
            poolHeads[127] = pool->next;
            poolHeads[127]->prev = nullptr;
            VAddr addr = (VAddr)pool;
            area->offset = addr;
            area->size = 4096;
            VirtualMemoryMapper::freePages(area->offset, 4096);
            freeMapping(area);
        }
    }

    bool allocNewPool() {
        if (kernelAreas == nullptr) {
            return false;
        }
        VAddr newPoolAddr = kernelAreas->offset;
        VirtualMemoryMapper::mapNewPages(newPoolAddr, 4096);
        kernelAreas->offset += 4096;
        kernelAreas->size -= 4096;
        if (kernelAreas->size == 0) {
            MemoryArea* next = kernelAreas->next;
            freeMemoryArea(kernelAreas);
            kernelAreas = next;
            kernelAreas->prev = nullptr;
        }
        MemoryAreaPool* pool = (MemoryAreaPool*)newPoolAddr;
        pool->init();
        pool->next = poolHeads[127];
        poolHeads[127] = pool;
        return true;
    }

    MemoryArea* findBestFit(Uint64 requestedSize) {
        MemoryArea *bestFit = nullptr, *current = kernelAreas;
        while (current != nullptr) {
            if (current->size >= requestedSize) {
                if (bestFit == nullptr) {
                    bestFit = current;
                } else if (bestFit->size > current->size) {
                    bestFit = current;
                }
            }
            current = current->next;
        }
        return bestFit;
    }

    VAddr KernelVirtualAllocator::getMapping(Uint64 size, PAddr physBase,
                                             bool managed) {
        MemoryArea* bestFit = findBestFit(size);
        if (bestFit == nullptr) {
            return 0;
        }
        VAddr offset = bestFit->offset;
        bestFit->offset += size;
        bestFit->size -= size;
        if (bestFit->size == 0) {
            cutNode(bestFit);
            freeMemoryArea(bestFit);
        }
        if (physBase == 0) {
            VirtualMemoryMapper::mapNewPages(offset, offset + size);
        } else {
            VirtualMemoryMapper::mapPages(offset, offset + size, physBase, managed);
        }
        freePools();
        return offset;
    }

    void freeRange(VAddr virtualAddr, Uint64 size) {
        if (size == 0) {
            return;
        }
        MemoryArea* area = allocMemoryArea();
        if (area == nullptr) {
            if (!allocNewPool()) {
                VAddr offset = virtualAddr;
                virtualAddr += 4096;
                size -= 4096;
                VirtualMemoryMapper::mapNewPages(offset, offset + 4096);
                MemoryAreaPool* pool = (MemoryAreaPool*)offset;
                pool->init();
                pool->next = poolHeads[127];
                poolHeads[127] = pool;
                if (size == 0) {
                    return;
                }
            }
            lastCheckedIndex = 127;
            // should not fail this time
            area = allocMemoryArea();
        }
        area->offset = virtualAddr;
        area->size = size;
        freeMapping(area);
        freePools();
    }

    void KernelVirtualAllocator::unmapAt(VAddr start, Uint64 size) {
        memory::VirtualMemoryMapper::freePages(start, start + size);
        freeRange(start, size);
    }

    void KernelVirtualAllocator::init() {
        if (!TempVirtualAllocator::isInitialized()) {
            panic("[TempVirtualAllocator] Dependency TempVirtualAllocator is "
                  "not satisfied");
        }
        if (!PhysAllocator::isInitialized()) {
            panic("[KernelVirtualAllocator] Dependency PhysAllocator is not "
                  "satisfied");
        }
        VAddr initAreaStart, initAreaEnd;
        initAreaStart = TempVirtualAllocator::getBrk();
        initAreaEnd = (VAddr)(((PageTable*)p4TableVirtualAddress)
                                  ->walkTo(511)
                                  ->walkTo(0)
                                  ->walkTo(0)
                                  ->walkTo(0));
        lastCheckedIndex = 127;
        freeRange(initAreaStart, initAreaEnd - initAreaStart);
        initialized = true;
    }

}; // namespace memory