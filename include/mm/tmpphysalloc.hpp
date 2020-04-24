#ifndef __TMP_PHYS_ALLOC_HPP_INCLUDED__
#define __TMP_PHYS_ALLOC_HPP_INCLUDED__

#include <mm/memoryinfo.hpp>
#include <mm/physbase.hpp>
#include <utils.hpp>

namespace memory {
    class TempPhysAllocator {
        static memory::MemoryMapEntry *m_currentEntry;
        static PAddr m_currentPhysAddr;
        static uint64_t m_areaUsed;
        static bool m_initialized;

        static bool afterCurrentMemoryArea();
        static bool beforeCurrentMemoryArea();
        static void AdjustMemoryArea();
        static bool CheckMultibootOverlap();

    public:
        static void init();
        INLINE static bool isInitialized() { return m_initialized; }
        static PAddr getFirstUnusedFrame();
        static PAddr newFrame();
    };
} // namespace memory

#endif