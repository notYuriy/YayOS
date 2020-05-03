#include <mm/kvmmngr.hpp>
#include <mm/vmmap.hpp>

namespace memory {

    bool KernelVirtualAllocator::m_initialized = false;

    static_assert(sizeof(MemoryArea) == 32);

    uint64_t KernelVirtualAllocator::m_lastCheckedIndex;
    MemoryAreaPool *KernelVirtualAllocator::m_poolHeads[128];
    MemoryArea *KernelVirtualAllocator::m_kernelAreas;
    proc::Mutex KernelVirtualAllocator::m_kvmmngrMutex;

    static_assert(sizeof(MemoryAreaPool) == 4096);

    void KernelVirtualAllocator::cutPool(MemoryAreaPool *pool) {
        if (pool->prev == nullptr) {
            m_poolHeads[pool->count] = pool->next;
        } else {
            pool->prev->next = pool->next;
        }
        if (pool->next != nullptr) {
            pool->next->prev = pool->prev;
        }
    }

    void KernelVirtualAllocator::insertPool(MemoryAreaPool *pool) {
        pool->prev = nullptr;
        pool->next = m_poolHeads[pool->count];
        if (pool->next != nullptr) {
            pool->next->prev = pool;
        } else {
            m_poolHeads[pool->count] = pool;
        }
    }

    MemoryArea *KernelVirtualAllocator::allocMemoryArea() {
        for (; m_lastCheckedIndex < 128ULL; ++m_lastCheckedIndex) {
            if (m_poolHeads[m_lastCheckedIndex] == nullptr) {
                continue;
            }
            MemoryAreaPool *pool = m_poolHeads[m_lastCheckedIndex];
            cutPool(pool);
            MemoryArea *result = pool->allocNode();
            insertPool(pool);
            if (m_lastCheckedIndex != 1) {
                m_lastCheckedIndex--;
            }
            result->next = nullptr;
            result->prev = nullptr;
            return result;
        }
        return nullptr;
    }

    void KernelVirtualAllocator::freeMemoryArea(MemoryArea *node) {
        MemoryAreaPool *pool =
            (MemoryAreaPool *)alignDown((uint64_t)node, 4096);
        cutPool(pool);
        pool->freeNode(node);
        insertPool(pool);
        if (pool->count < m_lastCheckedIndex) {
            m_lastCheckedIndex = pool->count;
        }
        return;
    }

    void KernelVirtualAllocator::insertBefore(MemoryArea *area,
                                              MemoryArea *point) {
        MemoryArea *prev = point->prev;
        if (prev != nullptr) {
            prev->next = area;
        } else {
            m_kernelAreas = area;
        }
        area->prev = prev;
        area->next = point;
        point->prev = area;
    }

    void KernelVirtualAllocator::cutNode(MemoryArea *area) {
        if (area->prev != nullptr) {
            area->prev->next = area->next;
        }
        if (area->next != nullptr) {
            area->next->prev = area->prev;
        }
    }

    void KernelVirtualAllocator::mergeAdjacent(MemoryArea *area) {
        bool mergeLeft = false, mergeRight = false;
        if (area->prev != nullptr) {
            mergeLeft = (area->prev->offset + area->prev->size == area->offset);
        }
        if (area->next != nullptr) {
            mergeRight = (area->offset + area->size == area->next->offset);
        }
        if (mergeLeft) {
            MemoryArea *prev = area->prev;
            if (area->prev->prev == nullptr) {
                m_kernelAreas = area;
            }
            area->offset = area->prev->offset;
            area->size += area->prev->size;
            cutNode(prev);
            freeMemoryArea(prev);
        }
        if (mergeRight) {
            MemoryArea *next = area->next;
            area->size += area->next->size;
            cutNode(next);
            freeMemoryArea(next);
        }
    }

    void KernelVirtualAllocator::freeMapping(MemoryArea *area) {
        MemoryArea *current = m_kernelAreas;
        if (current == nullptr) {
            m_kernelAreas = area;
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

    void KernelVirtualAllocator::freePools() {
        while (true) {
            if (m_poolHeads[127] == nullptr) {
                return;
            }
            if (m_lastCheckedIndex == 127 &&
                m_poolHeads[127]->next == nullptr) {
                return;
            }
            MemoryArea *area = allocMemoryArea();
            MemoryAreaPool *pool = m_poolHeads[127];
            m_poolHeads[127] = pool->next;
            m_poolHeads[127]->prev = nullptr;
            vaddr_t addr = (vaddr_t)pool;
            area->offset = addr;
            area->size = 4096;
            VirtualMemoryMapper::freePages(area->offset, 4096);
            freeMapping(area);
        }
    }

    bool KernelVirtualAllocator::allocNewPool() {
        if (m_kernelAreas == nullptr) {
            return false;
        }
        vaddr_t newPoolAddr = m_kernelAreas->offset;
        if (!VirtualMemoryMapper::mapNewPages(newPoolAddr, 4096)) {
            return false;
        }
        m_kernelAreas->offset += 4096;
        m_kernelAreas->size -= 4096;
        if (m_kernelAreas->size == 0) {
            MemoryArea *next = m_kernelAreas->next;
            freeMemoryArea(m_kernelAreas);
            m_kernelAreas = next;
            m_kernelAreas->prev = nullptr;
        }
        MemoryAreaPool *pool = (MemoryAreaPool *)newPoolAddr;
        pool->init();
        pool->next = m_poolHeads[127];
        m_poolHeads[127] = pool;
        return true;
    }

    MemoryArea *KernelVirtualAllocator::findBestFit(uint64_t requestedSize) {
        MemoryArea *bestFit = nullptr, *current = m_kernelAreas;
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

    vaddr_t KernelVirtualAllocator::getMapping(uint64_t size, paddr_t physBase,
                                               uint64_t flags) {
        m_kvmmngrMutex.lock();
        MemoryArea *bestFit = findBestFit(size);
        if (bestFit == nullptr) {
            m_kvmmngrMutex.unlock();
            return 0;
        }
        vaddr_t offset = bestFit->offset;
        bestFit->offset += size;
        bestFit->size -= size;
        if (bestFit->size == 0) {
            cutNode(bestFit);
            freeMemoryArea(bestFit);
        }
        if (physBase == 0) {
            if (!VirtualMemoryMapper::mapNewPages(offset, offset + size)) {
                m_kvmmngrMutex.unlock();
                return 0;
            }
        } else {
            if (!VirtualMemoryMapper::mapPages(offset, offset + size, physBase,
                                               flags)) {
                m_kvmmngrMutex.unlock();
                return 0;
            }
        }
        freePools();
        m_kvmmngrMutex.unlock();
        return offset;
    }

    void KernelVirtualAllocator::freeRange(vaddr_t virtualAddr, uint64_t size) {
        if (size == 0) {
            return;
        }
        MemoryArea *area = allocMemoryArea();
        if (area == nullptr) {
            if (!allocNewPool()) {
                vaddr_t offset = virtualAddr;
                virtualAddr += 4096;
                size -= 4096;
                VirtualMemoryMapper::mapNewPages(offset, offset + 4096);
                MemoryAreaPool *pool = (MemoryAreaPool *)offset;
                pool->init();
                pool->next = m_poolHeads[127];
                m_poolHeads[127] = pool;
                if (size == 0) {
                    return;
                }
            }
            m_lastCheckedIndex = 127;
            // should not fail this time
            area = allocMemoryArea();
        }
        area->offset = virtualAddr;
        area->size = size;
        freeMapping(area);
        freePools();
    }

    void KernelVirtualAllocator::unmapAt(vaddr_t start, uint64_t size) {
        m_kvmmngrMutex.lock();
        memory::VirtualMemoryMapper::freePages(start, start + size);
        freeRange(start, size);
        m_kvmmngrMutex.unlock();
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
        vaddr_t initAreaStart, initAreaEnd;
        initAreaStart = TempVirtualAllocator::getBrk();
        initAreaEnd = (vaddr_t)(((PageTable *)p4TableVirtualAddress)
                                    ->walkTo(511)
                                    ->walkTo(0)
                                    ->walkTo(0)
                                    ->walkTo(0));
        m_lastCheckedIndex = 127;
        freeRange(initAreaStart, initAreaEnd - initAreaStart);
        m_kvmmngrMutex.init();
        m_initialized = true;
    }

}; // namespace memory