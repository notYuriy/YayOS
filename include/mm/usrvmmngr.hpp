#ifndef __USRVMMNGR_HPP_INCLUDED__
#define __USRVMMNGR_HPP_INCLUDED__

#include <mm/kheap.hpp>
#include <mm/vmbase.hpp>
#include <mm/vmmap.hpp>
#include <utils.hpp>

namespace memory {
    struct UserVirtualMemoryArea {
        UserVirtualMemoryArea *prev, *next;
        memory::vaddr_t start;
        uint64_t size;
    };
    class UserVirtualAllocator {
        UserVirtualMemoryArea *m_head;
        UserVirtualAllocator();
        memory::vaddr_t alloc(uint64_t size);
        bool reserve(memory::vaddr_t addr, uint64_t size);
        bool free(memory::vaddr_t addr, uint64_t size);
    };
}; // namespace memory

#endif