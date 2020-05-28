#ifndef __COW_HPP_INCLUDED__
#define __COW_HPP_INCLUDED__

#include <memory/vmbase.hpp>
#include <utils.hpp>

namespace memory {
    class CoW {
        static bool m_initialized;
        static void disposePageTableRec(uint64_t level, PageTable *entry);
        static void cowPresent(PageTable *table);

    public:
        INLINE static bool isInitialized() { return m_initialized; }
        static void init();
        static uint64_t clonePageTable();
        static uint64_t newPageTable();
        static uint64_t markPageTable();
        static void deallocateUserMemory();
    };
}; // namespace memory

#endif