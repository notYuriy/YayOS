#ifndef __VMMAP_HPP_INLCUDED__
#define __VMMAP_HPP_INCLUDED__

#include <mm/physalloc.hpp>
#include <mm/vmbase.hpp>

namespace memory {
    class VirtualMemoryMapper {
        static void mapNewPageAt(VAddr addr, PAddr physAddr, Uint64 flags);
        static void freePageAt(VAddr addr);

    public:
        static void mapNewPages(VAddr start, VAddr end);
        static void mapPages(VAddr start, VAddr end, PAddr physStart,
                             Uint64 flags);
        // precondition: every single VAddr from start to end is mapped
        static void freePages(VAddr start, VAddr end);
    };
}; // namespace memory

#endif