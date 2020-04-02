#include <kheap.hpp>

namespace memory {
    bool KernelHeap::initialized;
    void* KernelHeap::alloc(Uint64 size) {}
    void KernelHeap::free(void* loc) {}
}; // namespace memory