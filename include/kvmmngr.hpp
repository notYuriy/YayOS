#ifndef __KVMMNGR_HPP_INCLUDED__
#define __KVMMNGR_HPP_INCLUDED__

#include <physbase.hpp>
#include <utils.hpp>
#include <vmbase.hpp>

namespace memory {
    class KernelVirtualAllocator {
        static bool initialized;

    public:
        static VAddr getMapping(Uint64 size, PAddr physBase, bool managed);
        static void unmapAt(VAddr virtualAddr, Uint64 size);
        static void init();
        INLINE static bool isInitialized() { return initialized; }
    };
}; // namespace memory

#endif