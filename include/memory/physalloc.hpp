#ifndef __PHYS_ALLOC_HPP_INCLUDED__
#define __PHYS_ALLOC_HPP_INCLUDED__

#include <memory/memoryinfo.hpp>
#include <memory/physbase.hpp>
#include <memory/tmpphysalloc.hpp>
#include <memory/tmpvalloc.hpp>
#include <memory/vmbase.hpp>
#include <utils.hpp>

namespace memory {
    struct PhysicalPageInfo {
        uint64_t refCount;
        uint64_t mapCount;
    };

    class PhysAllocator {
        static bool m_initialized;
        static uint64_t *m_bitmap;
        static PhysicalPageInfo *m_pageInfo;
        static uint64_t m_pagesCount;
        static uint64_t m_bitmapSize;
        static uint64_t m_leastUncheckedIndex;

        static void bitmapClearIndexRange(uint64_t start, uint64_t end);
        static void bitmapSetIndexRange(uint64_t start, uint64_t end);
        static void bitmapClearRange(paddr_t base, paddr_t limit);
        static void bitmapSetRange(paddr_t base, paddr_t limit);

    public:
        static void init();
        INLINE static bool isInitialized() { return m_initialized; }
        static paddr_t newPage(vaddr_t vaddrHint = 0);
        static void incrementRefCount(paddr_t addr);
        static paddr_t copyOnWrite(paddr_t orig, vaddr_t addrHint = 0);
        static void freePage(paddr_t addr);
        static void freePages(paddr_t addr, uint64_t count);
        static void incrementMapCount(paddr_t addr);
        static bool decrementMapCount(paddr_t addr);
    };
} // namespace memory

#endif