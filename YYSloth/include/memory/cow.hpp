#ifndef __COW_HPP_INCLUDED__
#define __COW_HPP_INCLUDED__

#include <memory/vmbase.hpp>
#include <utils.hpp>

namespace memory {
    class CoW {
        static bool m_initialized;
        static void disposePageTableRec(uint64_t level, PageTable *entry);

        static void markAsCoWRec(uint64_t level, PageTable *table);
        static void markAsCoW();

    public:
        INLINE static bool isInitialized() { return m_initialized; }
        static void init();
        static uint64_t clonePageTable();
        static uint64_t newPageTable();
        static uint64_t markPageTable();
        static void deallocateUserMemory();
        static PageTable *checkAndMoveToNext(PageTable *current, vind_t index,
                                             uint64_t errorCode);
    };
}; // namespace memory

#endif