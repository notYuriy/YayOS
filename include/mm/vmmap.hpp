#ifndef __VMMAP_HPP_INCLUDED__
#define __VMMAP_HPP_INCLUDED__

#include <mm/physalloc.hpp>
#include <mm/vmbase.hpp>

namespace memory {
    class VirtualMemoryMapper {
        static bool mapNewPageAt(vaddr_t addr, paddr_t physAddr,
                                 uint64_t flags);
        static bool freePageAt(vaddr_t addr);

    public:
        static bool mapNewPages(vaddr_t start, vaddr_t end);
        static bool mapPages(vaddr_t start, vaddr_t end, paddr_t physStart,
                             uint64_t flags);
        // precondition: every single VAddr from start to end is mapped
        static bool freePages(vaddr_t start, vaddr_t end);
    };
}; // namespace memory

#endif