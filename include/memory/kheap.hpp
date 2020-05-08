
#ifndef __KHEAP_HPP_INCLUDED__
#define __KHEAP_HPP_INCLUDED__

#include <proc/mutex.hpp>
#include <utils.hpp>

namespace memory {

    const uint64_t maxSizeForSlubs = 2000;
    constexpr uint64_t poolsSizesCount = maxSizeForSlubs / 16;

    class KernelHeap {
        static bool m_initialized;
        static proc::Mutex m_mutex;

        static struct SmallObjectPool **poolHeadsArray[poolsSizesCount];
        static uint64_t poolsMaxCount[poolsSizesCount];
        static uint64_t poolsLastCheckedIndices[poolsSizesCount];

        static void cutPoolFrom(struct SmallObjectPool *pool,
                                uint64_t sizeIndex, uint64_t headsIndex);
        static void insertPoolTo(struct SmallObjectPool *pool,
                                 uint64_t sizeIndex, uint64_t headsIndex);
        static bool isSlubEmpty(uint64_t sizeIndex);
        static bool getNewPool(uint64_t size);
        static struct ObjectHeader *allocFromSlubs(uint64_t size);

    public:
        static void init();
        static void *alloc(uint64_t size);
        static void free(void *loc);
        INLINE static bool isInitialized() { return m_initialized; }
    };

}; // namespace memory

#endif