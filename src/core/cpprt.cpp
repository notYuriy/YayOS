#include <memory/kheap.hpp>
#include <utils.hpp>

extern "C" void __cxa_pure_virtual() {
    panic("[CppRuntime] Pure virtual function is called");
}

void executeCtors(void (**ctorsStart)(), void (**ctorsEnd)()) {
    do {
        if (*ctorsStart != nullptr) {
            (*ctorsStart)();
        }
    } while (ctorsStart++ != ctorsEnd);
}

void *operator new(size_t size) noexcept {
    return memory::KernelHeap::alloc(size);
}

void *operator new[](size_t size) noexcept {
    return memory::KernelHeap::alloc(size);
}

void operator delete(void *loc) { memory::KernelHeap::free(loc); }
void operator delete(void *loc, UNUSED unsigned long int size) {
    memory::KernelHeap::free(loc);
}
void operator delete[](void *loc) { memory::KernelHeap::free(loc); }
void operator delete[](void *loc, UNUSED unsigned long int size) {
    memory::KernelHeap::free(loc);
}