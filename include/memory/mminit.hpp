#ifndef __MM_INIT_HPP_INCLUDED__
#define __MM_INIT_HPP_INCLUDED__

#include <boot/multiboot.hpp>
#include <memory/kheap.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/memoryinfo.hpp>
#include <memory/physalloc.hpp>
#include <memory/tmpphysalloc.hpp>
#include <memory/tmpvalloc.hpp>
#include <memory/vmmap.hpp>

namespace memory {
    INLINE void init(uint64_t mbPointer) {
        memory::BootMemoryInfo::init(mbPointer);
        memory::TempPhysAllocator::init();
        memory::TempVirtualAllocator::init(KERNEL_MAPPING_BASE + 128 MB);
        memory::PhysAllocator::init();
        memory::KernelVirtualAllocator::init();
        memory::KernelHeap::init();
    }
} // namespace memory

#endif