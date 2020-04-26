#ifndef __VMMAP_HPP_INLCUDED__
#define __VMMAP_HPP_INCLUDED__

#include <mm/physalloc.hpp>
#include <mm/vmbase.hpp>

namespace memory {
    class VirtualMemoryMapper {
        static bool mapNewPageAt(VAddr addr, PAddr physAddr, uint64_t flags);
        static bool freePageAt(VAddr addr);

    public:
        static bool mapNewPages(VAddr start, VAddr end);
        static bool mapPages(VAddr start, VAddr end, PAddr physStart,
                             uint64_t flags);
        // precondition: every single VAddr from start to end is mapped
        static bool freePages(VAddr start, VAddr end);
    };
}; // namespace memory

#endif