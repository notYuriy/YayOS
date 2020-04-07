#ifndef __KHEAP_HPP_INCLUDED__
#define __KHEAP_HPP_INCLUDED__

#include <utils.hpp>
#include <spinlock.hpp>

namespace memory {

    struct ObjectHeader;
    struct SmallObjectPool;

    const Uint64 maxSizeForSlubs = 2000;
    constexpr Uint64 poolsSizesCount = maxSizeForSlubs / 16;

    class KernelHeapArena {
        SmallObjectPool** poolHeadsArray[poolsSizesCount];
        Uint64 poolsMaxCount[poolsSizesCount];
        Uint64 poolsLastCheckedIndices[poolsSizesCount];
        Uint64 arenaId;
        lock::Spinlock lock;

        void cutPoolFrom(SmallObjectPool* pool, Uint64 sizeIndex,
                         Uint64 headsIndex);
        void insertPoolTo(SmallObjectPool* pool, Uint64 sizeIndex,
                          Uint64 headsIndex);
        bool isSlubEmpty(Uint64 sizeIndex);
        ObjectHeader* allocFromSlubs(Uint64 size);
        bool getNewPool(Uint64 size);


    public:
        void init(Uint64 id);
        void* alloc(Uint64 size);
        void free(void* loc);
    };

    class KernelHeap {
        static bool initialized;
        static KernelHeapArena* arenas;
        INLINE static Uint64 getCoreHeapId() {
            return 0;
        }
        INLINE static Uint64 getCoresCount() {
            return 1;
        }
    public:
        INLINE static bool isInitialized() { return initialized; }
        static void init();
        static void* alloc(Uint64 size); 
        static void free(void* ptr);
    };

}; // namespace memory

// compiler requires size_t here, so we can't use Uint64

INLINE void* operator new(size_t size) {
    return memory::KernelHeap::alloc(size);
}

INLINE void* operator new[](size_t size) {
    return memory::KernelHeap::alloc(size);
}

INLINE void operator delete(void* loc) { memory::KernelHeap::free(loc); }
INLINE void operator delete(void* loc, UNUSED unsigned long int size) {
    memory::KernelHeap::free(loc);
}
INLINE void operator delete[](void* loc) { memory::KernelHeap::free(loc); }
INLINE void operator delete[](void* loc, UNUSED unsigned long int size) {
    memory::KernelHeap::free(loc);
}

#endif