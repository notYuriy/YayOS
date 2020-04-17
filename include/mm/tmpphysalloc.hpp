#ifndef __TMP_PHYS_ALLOC_HPP_INCLUDED__
#define __TMP_PHYS_ALLOC_HPP_INCLUDED__

#include <mm/memoryinfo.hpp>
#include <mm/physbase.hpp>
#include <utils.hpp>

namespace memory {
    class TempPhysAllocator {
        static memory::MemoryMapEntry *currentEntry;
        static PAddr currentPhysAddr;
        static Uint64 areaUsed;
        static bool initialized;

        static bool afterCurrentMemoryArea();
        static bool beforeCurrentMemoryArea();
        static void AdjustMemoryArea();
        static bool CheckMultibootOverlap();

    public:
        static void init();
        INLINE static bool isInitialized() { return initialized; }
        static PAddr getFirstUnusedFrame();
        static PAddr newFrame();
    };
} // namespace memory

#endif