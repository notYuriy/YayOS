#include <mm/tmpphysalloc.hpp>

namespace memory {
    memory::MemoryMapEntry* TempPhysAllocator::currentEntry;
    Uint64 TempPhysAllocator::areaUsed;
    PAddr TempPhysAllocator::currentPhysAddr;
    bool TempPhysAllocator::initialized;

    bool TempPhysAllocator::afterCurrentMemoryArea() {
        return currentEntry->limit + 4096 < currentPhysAddr;
    }

    bool TempPhysAllocator::beforeCurrentMemoryArea() {
        return currentEntry->base > currentPhysAddr;
    }

    void TempPhysAllocator::AdjustMemoryArea() {
        while (true) {
            if (!afterCurrentMemoryArea()) {
                if (beforeCurrentMemoryArea()) {
                    currentPhysAddr = alignUp(currentEntry->base, 4096);
                }
                if (currentEntry->type ==
                    multiboot::MemoryMapEntryType::Available) {
                    break;
                }
            }
            currentEntry++;
            areaUsed++;
            if (areaUsed >= memory::BootMemoryInfo::mmapEntriesCount) {
                panic(
                    "[TempPhysAllocator] Can't find any available memory area");
            }
        }
    }

    bool TempPhysAllocator::CheckMultibootOverlap() {
        if (currentPhysAddr >= memory::BootMemoryInfo::multibootBase &&
            (currentPhysAddr < memory::BootMemoryInfo::multibootLimit)) {
            currentPhysAddr = memory::BootMemoryInfo::multibootLimit;
            return true;
        }
        return false;
    }

    void TempPhysAllocator::init() {
        if (!memory::BootMemoryInfo::isInitialized()) {
            panic("[TempPhysAllocator] BootMemoryInfo is not initialized\n\r");
        }
        currentPhysAddr = memory::BootMemoryInfo::kernelLimit;
        currentEntry = memory::BootMemoryInfo::mmapEntries;
        if (memory::BootMemoryInfo::mmapEntriesCount < 1) {
            panic("[TempPhysAllocator] No memory areas found\n\r");
        }
        areaUsed = 0;
        AdjustMemoryArea();
        CheckMultibootOverlap();
        AdjustMemoryArea();
        initialized = true;
    }

    PAddr TempPhysAllocator::getFirstUnusedFrame() { return currentPhysAddr; }

    PAddr TempPhysAllocator::newFrame() {
        AdjustMemoryArea();
        bool overlapped = CheckMultibootOverlap();
        if (overlapped) {
            AdjustMemoryArea();
        }
        PAddr result = currentPhysAddr;
        currentPhysAddr += 4096;
        return result;
    }

} // namespace memory