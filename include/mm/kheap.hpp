#ifndef __KHEAP_HPP_INCLUDED__
#define __KHEAP_HPP_INCLUDED__

#include <proc/spinlock.hpp>
#include <utils.hpp>

namespace memory {

    struct ObjectHeader;
    struct SmallObjectPool;

    const uint64_t maxSizeForSlubs = 2000;
    constexpr uint64_t poolsSizesCount = maxSizeForSlubs / 16;

    class KernelHeapArena {
        SmallObjectPool **m_poolHeadsArray[poolsSizesCount];
        uint64_t m_poolsMaxCount[poolsSizesCount];
        uint64_t m_poolsLastCheckedIndices[poolsSizesCount];
        uint64_t arenaId;
        proc::Spinlock lock;

        void cutPoolFrom(SmallObjectPool *pool, uint64_t sizeIndex,
                         uint64_t headsIndex);
        void insertPoolTo(SmallObjectPool *pool, uint64_t sizeIndex,
                          uint64_t headsIndex);
        bool isSlubEmpty(uint64_t sizeIndex);
        ObjectHeader *allocFromSlubs(uint64_t size);
        bool getNewPool(uint64_t size);

    public:
        void init(uint64_t id);
        void *alloc(uint64_t size);
        void free(void *loc);
    };

    class KernelHeap {
        static bool m_initialized;
        static KernelHeapArena *m_arenas;
        INLINE static uint64_t getCoreHeapId() { return 0; }
        INLINE static uint64_t getCoresCount() { return 1; }

    public:
        INLINE static bool isInitialized() { return m_initialized; }
        static void init();
        static void *alloc(uint64_t size);
        static void free(void *ptr);
    };

}; // namespace memory

// compiler requires size_t here, so we can't use Uint64

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