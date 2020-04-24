#ifndef __PHYS_ALLOC_HPP_INCLUDED__
#define __PHYS_ALLOC_HPP_INCLUDED__

#include <mm/memoryinfo.hpp>
#include <mm/physbase.hpp>
#include <mm/tmpphysalloc.hpp>
#include <mm/tmpvalloc.hpp>
#include <mm/vmbase.hpp>
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
        static void bitmapClearRange(PAddr base, PAddr limit);
        static void bitmapSetRange(PAddr base, PAddr limit);

    public:
        static void init();
        INLINE static bool isInitialized() { return m_initialized; }
        static PAddr newPage(VAddr vaddrHint = 0);
        static void incrementRefCount(PAddr addr);
        static PAddr copyOnWrite(PAddr orig, VAddr addrHint = 0);
        static void freePage(PAddr addr);
        static void freePages(PAddr addr, uint64_t count);
        static void incrementMapCount(PAddr addr);
        static bool decrementMapCount(PAddr addr);
    };
} // namespace memory

#endif