#ifndef __MM_INIT_HPP_INCLUDED__
#define __MM_INIT_HPP_INCLUDED__

#include <kheap.hpp>
#include <kvmmngr.hpp>
#include <memoryinfo.hpp>
#include <multiboot.hpp>
#include <physalloc.hpp>
#include <tmpphysalloc.hpp>
#include <tmpvalloc.hpp>
#include <vmmap.hpp>

namespace memory {
    INLINE void init(Uint64 mbPointer) {
        memory::BootMemoryInfo::init(mbPointer);
        memory::TempPhysAllocator::init();
        memory::TempVirtualAllocator::init(KERNEL_MAPPING_BASE + 128 MB);
        memory::PhysAllocator::init();
        memory::KernelVirtualAllocator::init();
        memory::KernelHeap::init();
    }
} // namespace memory

#endif