#ifndef __UNIQUE_PTR_HPP_INCLUDED__
#define __UNIQUE_PTR_HPP_INCLUDED__

#include <mm/kheap.hpp>

namespace core {
    template <class T> class UniquePtr {
        T *ptr;

    public:
        INLINE T *get() { return ptr; }
        INLINE UniquePtr(decltype(nullptr) p) { ptr = p; }
        INLINE UniquePtr(T *p) { ptr = p; }
        INLINE ~UniquePtr() {
            if (ptr != nullptr) {
                delete ptr;
            }
            ptr = nullptr;
        }
        INLINE T *move() {
            T *result = ptr;
            ptr = nullptr;
            return result;
        }
    };
}; // namespace core

#endif