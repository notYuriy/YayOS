#include <physalloc.hpp>

namespace memory {
    bool PhysAllocator::initialized;
    Uint64* PhysAllocator::bitmap;
    PhysicalPageInfo* PhysAllocator::pageInfo;
    Uint64 PhysAllocator::pagesCount;
    Uint64 PhysAllocator::bitmapSize;
    Uint64 PhysAllocator::leastUncheckedIndex;

    INLINE void setBit(Uint64& num, Uint8 bit) { num |= (1ULL << bit); }
    INLINE void clearBit(Uint64& num, Uint8 bit) { num &= ~(1ULL << bit); }

    void PhysAllocator::bitmapClearIndexRange(Uint64 start, Uint64 end) {
        if (start >= pagesCount) {
            return;
        }
        if (end > pagesCount) {
            end = pagesCount;
        }
        Uint64 startBitmapIndex = start / 64;
        Uint64 endBitmapIndex = end / 64;
        if (startBitmapIndex == endBitmapIndex) {
            for (Uint64 bit = start % 64; bit < end % 64; ++bit) {
                clearBit(bitmap[startBitmapIndex], bit);
            }
        } else {
            for (Uint64 bit = start % 64; bit < 64; ++bit) {
                clearBit(bitmap[startBitmapIndex], bit);
            }
            for (Uint64 bit = 0; bit < end % 64; ++bit) {
                clearBit(bitmap[endBitmapIndex], bit);
            }
            for (Uint64 i = startBitmapIndex + 1; i < endBitmapIndex; ++i) {
                bitmap[i] = 0;
            }
        }
    } // namespace memory

    void PhysAllocator::bitmapSetIndexRange(Uint64 start, Uint64 end) {
        if (start >= pagesCount) {
            return;
        }
        if (end > pagesCount) {
            end = pagesCount;
        }
        Uint64 startBitmapIndex = start / 64;
        Uint64 endBitmapIndex = end / 64;
        if (startBitmapIndex == endBitmapIndex) {
            for (Uint64 bit = start % 64; bit < end % 64; ++bit) {
                setBit(bitmap[startBitmapIndex], bit);
            }
        } else {
            for (Uint64 bit = start % 64; bit < 64; ++bit) {
                setBit(bitmap[startBitmapIndex], bit);
            }
            for (Uint64 bit = 0; bit < end % 64; ++bit) {
                setBit(bitmap[endBitmapIndex], bit);
            }
            for (Uint64 i = startBitmapIndex + 1; i < endBitmapIndex; ++i) {
                bitmap[i] = UINT64_MAX;
            }
        }
    }

    INLINE void PhysAllocator::bitmapSetRange(PAddr base, PAddr limit) {
        PhysAllocator::bitmapSetIndexRange(base / 4096, limit / 4096);
    }

    INLINE void PhysAllocator::bitmapClearRange(PAddr base, PAddr limit) {
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

        pagesCount = BootMemoryInfo::upperLimit / 4096;
        bitmapSize = alignUp(pagesCount, 64) / 64;
        bitmap = (Uint64*)TempVirtualAllocator::valloc(bitmapSize * 8);
        pageInfo = (PhysicalPageInfo*)TempVirtualAllocator::valloc(
            pagesCount * sizeof(PhysicalPageInfo));
        bitmapSetRange(0, TempPhysAllocator::getFirstUnusedFrame());
        for (Uint64 i = 0; i < TempPhysAllocator::getFirstUnusedFrame();
             i += 4096) {
            // prevent deallocation of currently used tables
            pageInfo[i / 4096] = {.refCount = 1, .mapCount = 1};
        }
        bitmapClearRange(TempPhysAllocator::getFirstUnusedFrame(),
                         pagesCount * 4096);
        for (Uint64 i = 0; i < BootMemoryInfo::mmapEntriesCount; ++i) {
            MemoryMapEntry& entry = BootMemoryInfo::mmapEntries[i];
            if (entry.type != multiboot::MemoryMapEntryType::Available) {
                bitmapSetRange(entry.base, entry.limit);
            }
        }
        leastUncheckedIndex = 0;
        initialized = 1;
    }

    PAddr PhysAllocator::newPage(UNUSED VAddr addrHint) {
        PAddr addr = leastUncheckedIndex * 64 * 4096;
        for (Uint64 i = leastUncheckedIndex; i < bitmapSize;
             ++i, addr += 64 * 4096) {
            if (~bitmap[i] == 0) {
                continue;
            }
            Uint8 index = bitScanForward(~bitmap[i]);
            pageInfo[64 * i + index] = {.refCount = 1, .mapCount = 0};
            setBit(bitmap[i], index);
            leastUncheckedIndex = i;
            return addr + (Uint64)index * 4096;
        }
        return 0;
    }

    PAddr PhysAllocator::copyOnWrite(PAddr addr, VAddr addrHint) {
        if (pageInfo[addr / 4096].refCount == 1) {
            return addr;
        } else {
            pageInfo[addr / 4096].refCount--;
            return newPage(addrHint);
        }
    }

    void PhysAllocator::freePage(PAddr addr) {
        if (--pageInfo[addr / 4096].refCount == 0) {
            clearBit(bitmap[addr / (4096ULL * 64ULL)], (addr / 4096) % 64);
            leastUncheckedIndex = 0;
        }
    }

    void PhysAllocator::freePages(PAddr addr, Uint64 count) {
        PAddr p = addr;
        for (Uint64 i = 0; i < count; ++i, p += 4096) {
            freePage(p);
        }
    }

    void PhysAllocator::incrementRefCount(PAddr addr) {
        pageInfo[addr / 4096].refCount++;
    }

    void PhysAllocator::incrementMapCount(PAddr addr) {
        pageInfo[addr / 4096].mapCount++;
    }

    bool PhysAllocator::decrementMapCount(PAddr addr) {
        return --(pageInfo[addr / 4096].mapCount) == 0;
    }

} // namespace memory