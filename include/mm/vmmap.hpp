#ifndef __VMMAP_HPP_INLCUDED__
#define __VMMAP_HPP_INCLUDED__

#include <physalloc.hpp>
#include <vmbase.hpp>

namespace memory {
    class VirtualMemoryMapper {
        static void mapNewPageAt(VAddr addr, PAddr physAddr, bool managed);
        static void freePageAt(VAddr addr);

    public:
        static void mapNewPages(VAddr start, VAddr end);
        static void mapPages(VAddr start, VAddr end, PAddr physStart,
                             bool managed = true);
        // precondition: every single VAddr from start to end is mapped
        static void freePages(VAddr start, VAddr end);
    };
}; // namespace memory

#endif