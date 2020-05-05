#ifndef __USRVMMNGR_HPP_INCLUDED__
#define __USRVMMNGR_HPP_INCLUDED__

#include <mm/kheap.hpp>
#include <mm/vmbase.hpp>
#include <mm/vmmap.hpp>
#include <proc/mutex.hpp>
#include <utils.hpp>

namespace memory {
    struct UserVirtualMemoryArea {
        UserVirtualMemoryArea *prev, *next;
        memory::vaddr_t start;
        uint64_t size;
        UserVirtualMemoryArea(memory::vaddr_t start, uint64_t size);
        bool in(memory::vaddr_t addr);
    };
    class UserVirtualAllocator {
        UserVirtualMemoryArea *m_head;
        proc::Mutex mutex;

        UserVirtualMemoryArea *findBestFit(uint64_t size);
        void cut(UserVirtualMemoryArea *area);
        bool cutFrom(UserVirtualMemoryArea *area, memory::vaddr_t start,
                     uint64_t size);
        UserVirtualMemoryArea *findLastBefore(memory::vaddr_t start);

    public:
        UserVirtualAllocator() = default;
        memory::vaddr_t alloc(uint64_t size);
        bool reserve(memory::vaddr_t addr, uint64_t size);
        bool free(memory::vaddr_t addr, uint64_t size);
        friend UserVirtualAllocator *newUserVirtualAllocator();
    };
    UserVirtualAllocator *newUserVirtualAllocator();

}; // namespace memory

#endif