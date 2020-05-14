#include <memory/physalloc.hpp>
#include <proc/mutex.hpp>

namespace memory {
    bool PhysAllocator::m_initialized;
    uint64_t *PhysAllocator::m_bitmap;
    PhysicalPageInfo *PhysAllocator::m_pageInfo;
    uint64_t PhysAllocator::m_pagesCount;
    uint64_t PhysAllocator::m_bitmapSize;
    uint64_t PhysAllocator::m_leastUncheckedIndex;
    proc::Mutex physMutex;

    INLINE void setBit(uint64_t &num, uint8_t bit) { num |= (1ULL << bit); }
    INLINE void clearBit(uint64_t &num, uint8_t bit) { num &= ~(1ULL << bit); }

    void PhysAllocator::bitmapClearIndexRange(uint64_t start, uint64_t end) {
        if (start >= m_pagesCount) {
            return;
        }
        if (end > m_pagesCount) {
            end = m_pagesCount;
        }
        uint64_t startBitmapIndex = start / 64;
        uint64_t endBitmapIndex = end / 64;
        if (startBitmapIndex == endBitmapIndex) {
            for (uint64_t bit = start % 64; bit < end % 64; ++bit) {
                clearBit(m_bitmap[startBitmapIndex], bit);
            }
        } else {
            for (uint64_t bit = start % 64; bit < 64; ++bit) {
                clearBit(m_bitmap[startBitmapIndex], bit);
            }
            for (uint64_t bit = 0; bit < end % 64; ++bit) {
                clearBit(m_bitmap[endBitmapIndex], bit);
            }
            for (uint64_t i = startBitmapIndex + 1; i < endBitmapIndex; ++i) {
                m_bitmap[i] = 0;
            }
        }
    } // namespace memory

    void PhysAllocator::bitmapSetIndexRange(uint64_t start, uint64_t end) {
        if (start >= m_pagesCount) {
            return;
        }
        if (end > m_pagesCount) {
            end = m_pagesCount;
        }
        uint64_t startBitmapIndex = start / 64;
        uint64_t endBitmapIndex = end / 64;
        if (startBitmapIndex == endBitmapIndex) {
            for (uint64_t bit = start % 64; bit < end % 64; ++bit) {
                setBit(m_bitmap[startBitmapIndex], bit);
            }
        } else {
            for (uint64_t bit = start % 64; bit < 64; ++bit) {
                setBit(m_bitmap[startBitmapIndex], bit);
            }
            for (uint64_t bit = 0; bit < end % 64; ++bit) {
                setBit(m_bitmap[endBitmapIndex], bit);
            }
            for (uint64_t i = startBitmapIndex + 1; i < endBitmapIndex; ++i) {
                m_bitmap[i] = UINT64_MAX;
            }
        }
    }

    INLINE void PhysAllocator::bitmapSetRange(paddr_t base, paddr_t limit) {
        PhysAllocator::bitmapSetIndexRange(base / 4096, limit / 4096);
    }

    INLINE void PhysAllocator::bitmapClearRange(paddr_t base, paddr_t limit) {
        PhysAllocator::bitmapClearIndexRange(base / 4096, limit / 4096);
    }

    void PhysAllocator::init() {
        if (!BootMemoryInfo::isInitialized()) {
            panic("[PhysAllocator] Dependency \"BootMemoryInfo\" is not "
                  "initialized");
        }
        if (!TempPhysAllocator::isInitialized()) {
            panic("[PhysAllocator] Dependency \"TempPhysAllocator\" is not "
                  "initialized");
        }
        if (!TempVirtualAllocator::isInitialized()) {
            panic("[PhysAllocator] Dependency \"TempVirtualAllocator\" is "
                  "not "
                  "initialized");
        }

        m_pagesCount = BootMemoryInfo::upperLimit / 4096;
        m_bitmapSize = alignUp(m_pagesCount, 64) / 64;
        m_bitmap = (uint64_t *)TempVirtualAllocator::valloc(m_bitmapSize * 8);
        m_pageInfo = (PhysicalPageInfo *)TempVirtualAllocator::valloc(
            m_pagesCount * sizeof(PhysicalPageInfo));
        bitmapSetRange(0, TempPhysAllocator::getFirstUnusedFrame());
        bitmapSetRange(alignDown(memory::BootMemoryInfo::multibootBase, 4096),
                       alignUp(memory::BootMemoryInfo::multibootLimit, 4096));
        bitmapSetRange(alignDown(memory::BootMemoryInfo::initrdBase, 4096),
                       alignUp(memory::BootMemoryInfo::initrdLimit, 4096));
        bitmapClearRange(TempPhysAllocator::getFirstUnusedFrame(),
                         m_pagesCount * 4096);
        for (uint64_t i = 0; i < BootMemoryInfo::mmapEntriesCount; ++i) {
            MemoryMapEntry &entry = BootMemoryInfo::mmapEntries[i];
            if (entry.type != multiboot::MemoryMapEntryType::Available) {
                bitmapSetRange(entry.base, entry.limit);
            }
        }
        m_leastUncheckedIndex = 0;
        m_initialized = true;
    }

    paddr_t PhysAllocator::newPage(UNUSED vaddr_t addrHint) {
        physMutex.lock();
        paddr_t addr = m_leastUncheckedIndex * 64 * 4096;
        for (uint64_t i = m_leastUncheckedIndex; i < m_bitmapSize;
             ++i, addr += 64 * 4096) {
            if (~m_bitmap[i] == 0) {
                continue;
            }
            uint8_t index = bitScanForward(~m_bitmap[i]);
            m_pageInfo[64 * i + index].refCount = 1;
            m_pageInfo[64 * i + index].mapCount = 1;
            setBit(m_bitmap[i], index);
            m_leastUncheckedIndex = i;
            physMutex.unlock();
            return addr + (uint64_t)index * 4096;
        }
        physMutex.unlock();
        return 0;
    }

    paddr_t PhysAllocator::copyOnWrite(paddr_t addr, vaddr_t addrHint) {
        if (m_pageInfo[addr / 4096].refCount == 1) {
            return addr;
        } else {
            m_pageInfo[addr / 4096].refCount--;
            return newPage(addrHint);
        }
    }

    void PhysAllocator::freePage(paddr_t addr) {
        physMutex.lock();
        if (--m_pageInfo[addr / 4096].refCount == 0) {
            clearBit(m_bitmap[addr / (4096ULL * 64ULL)],
                     (addr / 4096ULL) % 64ULL);
            m_leastUncheckedIndex = 0;
        }
        physMutex.unlock();
    }

    void PhysAllocator::freePages(paddr_t addr, uint64_t count) {
        paddr_t p = addr;
        for (uint64_t i = 0; i < count; ++i, p += 4096) {
            freePage(p);
        }
    }

    void PhysAllocator::incrementRefCount(paddr_t addr) {
        ++(m_pageInfo[addr / 4096].refCount);
    }

    void PhysAllocator::incrementMapCount(paddr_t addr) {
        ++(m_pageInfo[addr / 4096].mapCount);
    }

    bool PhysAllocator::decrementMapCount(paddr_t addr) {
        return --(m_pageInfo[addr / 4096].mapCount) == 0;
    }

} // namespace memory