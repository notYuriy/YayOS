#ifndef __VM_BASE_HPP_INCLUDED__
#define __VM_BASE_HPP_INCLUDED__

#include <mm/physbase.hpp>
#include <utils.hpp>

namespace memory {
    typedef uint64_t VAddr;
    typedef uint32_t VIndex;
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

    const uint64_t p4TableVirtualAddress = 01777777777777777770000ULL;
    const uint64_t pageTableEntryFlagsMask = 0b111111111111ULL;

    const uint64_t levelSizes[] = {4096ULL, 4096ULL * 512ULL,
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
                    bool managed : 1;
                    bool flag2 : 1;
                    bool flag3 : 1;
                };
                uint16_t lowFlags : 12;
            };
        };
    };
#pragma pack(0)

    const uint64_t defaultKernelFlags = (1 << 0) | (1 << 1) | (1 << 10);
    const uint64_t defaultUnmanagedFlags = (1 << 0) | (1 << 1);
    const uint64_t defaultVolatileDevFlags = (1 << 0) | (1 << 1) | (1 << 5);

    static_assert(sizeof(PageTableEntry) == 8);

#pragma pack(1)
    struct PageTable {
        PageTableEntry entries[512];
        PageTableEntry &operator[](uint16_t index) { return entries[index]; }

        INLINE PageTable *walkTo(VIndex index) {
            return (PageTable *)((((uint64_t)this) << 9ULL) |
                                 ((uint64_t)(index) << 12ULL));
        }

        PageTable *walkToWithTempAlloc(VIndex index);

        PageTable *walkToWithAlloc(VIndex index, PAddr currentAddr);
    };
#pragma pack(0)

    static_assert(sizeof(PageTable) == 4096);

} // namespace memory

#endif