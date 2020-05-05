#ifndef __UNIQUE_PTR_HPP_INCLUDED__
#define __UNIQUE_PTR_HPP_INCLUDED__

#include <mm/kheap.hpp>

namespace core {
    template <class T> class UniquePtr {
        T *ptr;

    public:
        INLINE T *get() { return ptr; }
        INLINE UniquePtr(T *p) { ptr = p; }
        INLINE ~UniquePtr() {
            if (ptr != nullptr) {
                delete ptr;
            }
            ptr = nullptr;
        }
        INLINE UniquePtr(UniquePtr &other) {
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        template <class R> INLINE operator R *() { return (R *)ptr; }
    };
}; // namespace core

#endif