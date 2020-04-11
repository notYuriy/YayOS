#ifndef __KVMMNGR_HPP_INCLUDED__
#define __KVMMNGR_HPP_INCLUDED__

#include <mm/physbase.hpp>
#include <utils.hpp>
#include <mm/vmbase.hpp>

namespace memory {
    class KernelVirtualAllocator {
        static bool initialized;

    public:
        static VAddr getMapping(Uint64 size, PAddr physBase, Uint64 flags);
        static void unmapAt(VAddr virtualAddr, Uint64 size);
        static void init();
        INLINE static bool isInitialized() { return initialized; }
    };
}; // namespace memory

#endif