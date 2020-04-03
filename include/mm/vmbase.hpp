#ifndef __VM_BASE_HPP_INCLUDED__
#define __VM_BASE_HPP_INCLUDED__

#include <physbase.hpp>
#include <utils.hpp>

namespace memory {
    typedef Uint64 VAddr;
    typedef Uint32 VIndex;
    extern "C" void vmbaseLoadP4(memory::PAddr p4addr);
    extern "C" void vmbaseInvalidateCache(memory::VAddr page);

    INLINE static VIndex getP4Index(VAddr addr) {
        return (addr >> 39ULL) & 0777ULL;
    }
    INLINE static VIndex getP3Index(VAddr addr) {
        return (addr >> 30ULL) & 0777ULL;
    }
    INLINE static VIndex getP2Index(VAddr addr) {
        return (addr >> 21ULL) & 0777ULL;
    }
    INLINE static VIndex getP1Index(VAddr addr) {
        return (addr >> 12ULL) & 0777ULL;
    }

    const Uint64 p4TableVirtualAddress = 01777777777777777770000ULL;
    const Uint64 pageTableEntryFlagsMask = 0b111111111111ULL;

    const Uint64 levelSizes[] = {4096ULL, 4096ULL * 512ULL,
                                 4096ULL * 512ULL * 512ULL,
                                 4096ULL * 512ULL * 512ULL * 512ULL,
                                 4096ULL * 512ULL * 512ULL * 512ULL * 512ULL};

    #pragma pack(1)
    union PageTableEntry {
        PAddr addr;
        struct {
            union {
                struct {
                    bool present : 1;
                    bool writable : 1;
                    bool userAccessible : 1;
                    bool writeThroughCaching : 1;
                    bool disableCache : 1;
                    bool accessed : 1;
                    bool dirty : 1;
                    bool hugePage : 1;
                    bool global : 1;
                    bool flag1 : 1;
                    bool flag2 : 1;
                    bool flag3 : 1;
                };
                Uint16 lowFlags : 12;
            };
        };
    };
    #pragma pack(0)

    static_assert(sizeof(PageTableEntry) == 8);

    #pragma pack(1)
    struct PageTable {
        PageTableEntry entries[512];
        PageTableEntry& operator[](Uint16 index) { return entries[index]; }

        INLINE PageTable* walkTo(VIndex index) {
            return (PageTable*)((((Uint64)this) << 9ULL) |
                                ((Uint64)(index) << 12ULL));
        }

        PageTable* walkToWithTempAlloc(VIndex index);

        PageTable* walkToWithAlloc(VIndex index, PAddr currentAddr);

    };
    #pragma pack(0)

    static_assert(sizeof(PageTable) == 4096);

} // namespace memory

#endif