#ifndef __KVMMNGR_HPP_INCLUDED__
#define __KVMMNGR_HPP_INCLUDED__

#include <mm/physbase.hpp>
#include <mm/vmbase.hpp>
#include <utils.hpp>

namespace memory {
    class KernelVirtualAllocator {
        static bool m_initialized;

    public:
        static VAddr getMapping(uint64_t size, PAddr physBase, uint64_t flags);
        static void unmapAt(VAddr virtualAddr, uint64_t size);
        static void init();
        INLINE static bool isInitialized() { return m_initialized; }
    };
}; // namespace memory

#endif