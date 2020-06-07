#ifndef __TMP_VALLOC_HPP_INCLUDED__
#define __TMP_VALLOC_HPP_INCLUDED__

#include <memory/physalloc.hpp>
#include <memory/vmbase.hpp>

namespace memory {
    class TempVirtualAllocator {
        static vaddr_t m_pageEnd;
        static vaddr_t m_unalignedEnd;
        static bool m_initialized;

    public:
        static void init(vaddr_t initMappingEnd);
        INLINE static bool isInitialized() { return m_initialized; }
        static void *valloc(uint64_t size);
        INLINE static vaddr_t getBrk() { return m_pageEnd; }
    };
}; // namespace memory

#endif