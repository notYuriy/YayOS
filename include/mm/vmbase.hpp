#ifndef __VM_BASE_HPP_INCLUDED__
#define __VM_BASE_HPP_INCLUDED__

#include <mm/physbase.hpp>
#include <utils.hpp>

namespace memory {
    typedef uint64_t vaddr_t;
    typedef uint32_t vind_t;
    extern "C" void vmbaseLoadP4(memory::paddr_t p4addr);
    extern "C" void vmbaseInvalidateCache(memory::vaddr_t page);

    INLINE static vind_t getP4Index(vaddr_t addr) {
        return (addr >> 39ULL) & 0777ULL;
    }
    INLINE static vind_t getP3Index(vaddr_t addr) {
        return (addr >> 30ULL) & 0777ULL;
    }
    INLINE static vind_t getP2Index(vaddr_t addr) {
        return (addr >> 21ULL) & 0777ULL;
    }
    INLINE static vind_t getP1Index(vaddr_t addr) {
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
        paddr_t addr;
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

    constexpr uint64_t DEFAULT_KERNEL_FLAGS =
        (1 << 0) | (1 << 1) | (1 << 9) | (1LLU << 63);
    constexpr uint64_t DEFAULT_UNMANAGED_FLAGS =
        (1 << 0) | (1 << 1) | (1LLU << 63);
    constexpr uint64_t DEFAULT_VOLATILE_DEV_FLAGS =
        (1 << 0) | (1 << 1) | (1 << 5) | (1LLU << 63);

    static_assert(sizeof(PageTableEntry) == 8);

#pragma pack(1)
    struct PageTable {
        PageTableEntry entries[512];
        PageTableEntry &operator[](uint16_t index) { return entries[index]; }

        INLINE PageTable *walkTo(vind_t index) {
            return (PageTable *)((((uint64_t)this) << 9ULL) |
                                 ((uint64_t)(index) << 12ULL));
        }

        PageTable *walkToWithTempAlloc(vind_t index);

        PageTable *walkToWithAlloc(vind_t index, paddr_t currentAddr,
                                   bool userAccessible);
    };
#pragma pack(0)

    static_assert(sizeof(PageTable) == 4096);

} // namespace memory

#endif