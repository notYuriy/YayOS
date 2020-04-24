#ifndef __TMP_VALLOC_HPP_INCLUDED__
#define __TMP_VALLOC_HPP_INCLUDED__

#include <mm/physalloc.hpp>
#include <mm/vmbase.hpp>

namespace memory {
    class TempVirtualAllocator {
        static VAddr m_pageEnd;
        static VAddr m_unalignedEnd;
        static bool m_initialized;

    public:
        static void init(VAddr initMappingEnd);
        INLINE static bool isInitialized() { return m_initialized; }
        static void *valloc(uint64_t size);
        INLINE static VAddr getBrk() { return m_pageEnd; }
    };
}; // namespace memory

#endif