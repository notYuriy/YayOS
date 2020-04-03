#ifndef __KHEAP_HPP_INCLUDED__
#define __KHEAP_HPP_INCLUDED__

#include <utils.hpp>

namespace memory {

    class KernelHeap {
        static bool initialized;

    public:
        static void init();
        static void* alloc(Uint64 size);
        static void free(void* loc);
        INLINE static bool isInitialized() { return initialized; }
    };

}; // namespace memory

//compiler requires size_t here, so we can't use Uint64

INLINE void* operator new(size_t size) {
    return memory::KernelHeap::alloc(size);
}
INLINE void* operator new[](size_t size) {
    return memory::KernelHeap::alloc(size);
}

INLINE void operator delete(void* loc) { memory::KernelHeap::free(loc); }

INLINE void operator delete[](void* loc) { memory::KernelHeap::free(loc); }

#endif