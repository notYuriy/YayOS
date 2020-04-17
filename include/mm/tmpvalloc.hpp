#ifndef __TMP_VALLOC_HPP_INCLUDED__
#define __TMP_VALLOC_HPP_INCLUDED__

#include <mm/physalloc.hpp>
#include <mm/vmbase.hpp>

namespace memory {
    class TempVirtualAllocator {
        static VAddr pageEnd;
        static VAddr unalignedEnd;
        static bool initialized;

    public:
        static void init(VAddr initMappingEnd);
        INLINE static bool isInitialized() { return initialized; }
        static void *valloc(Uint64 size);
        INLINE static VAddr getBrk() { return pageEnd; }
    };
}; // namespace memory

#endif