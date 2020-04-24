
#ifndef __KHEAP_HPP_INCLUDED__
#define __KHEAP_HPP_INCLUDED__

#include <utils.hpp>

namespace memory {

    class KernelHeap {
        static bool initialized;

    public:
        static void init();
        static void *alloc(uint64_t size);
        static void free(void *loc);
        INLINE static bool isInitialized() { return initialized; }
    };

}; // namespace memory

// compiler requires size_t here, so we can't use uint64_t

INLINE void *operator new(size_t size) {
    return memory::KernelHeap::alloc(size);
}

INLINE void *operator new[](size_t size) {
    return memory::KernelHeap::alloc(size);
}

INLINE void operator delete(void *loc) { memory::KernelHeap::free(loc); }
INLINE void operator delete(void *loc, UNUSED unsigned long int size) {
    memory::KernelHeap::free(loc);
}
INLINE void operator delete[](void *loc) { memory::KernelHeap::free(loc); }
INLINE void operator delete[](void *loc, UNUSED unsigned long int size) {
    memory::KernelHeap::free(loc);
}

#endif