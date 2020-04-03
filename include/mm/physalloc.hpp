#ifndef __PHYS_ALLOC_HPP_INCLUDED__
#define __PHYS_ALLOC_HPP_INCLUDED__

#include <memoryinfo.hpp>
#include <physbase.hpp>
#include <tmpphysalloc.hpp>
#include <tmpvalloc.hpp>
#include <utils.hpp>
#include <vmbase.hpp>

namespace memory {
    struct PhysicalPageInfo {
        Uint64 refCount;
        Uint64 mapCount;
    };

    class PhysAllocator {
        static bool initialized;
        static Uint64* bitmap;
        static PhysicalPageInfo* pageInfo;
        static Uint64 pagesCount;
        static Uint64 bitmapSize;
        static Uint64 leastUncheckedIndex;

        static void bitmapClearIndexRange(Uint64 start, Uint64 end);
        static void bitmapSetIndexRange(Uint64 start, Uint64 end);
        static void bitmapClearRange(PAddr base, PAddr limit);
        static void bitmapSetRange(PAddr base, PAddr limit);

    public:
        static void init();
        INLINE static bool isInitialized() { return initialized; }
        static PAddr newPage(VAddr vaddrHint = 0);
        static void incrementRefCount(PAddr addr);
        static PAddr copyOnWrite(PAddr orig, VAddr addrHint = 0);
        static void freePage(PAddr addr);
        static void freePages(PAddr addr, Uint64 count);
        static void incrementMapCount(PAddr addr);
        static bool decrementMapCount(PAddr addr);
    };
} // namespace memory

#endif