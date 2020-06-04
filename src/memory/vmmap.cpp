#include <memory/vmmap.hpp>

namespace memory {
    // remaps page with different flags if a page exists
    bool VirtualMemoryMapper::mapNewPageAt(vaddr_t addr, paddr_t physAddr,
                                           uint64_t flags) {
        bool userAccessible = (flags & (1LLU << 2)) != 0;
        PageTable *p4Table = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        vind_t p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        paddr_t p3addr, p2addr, p1addr;
        PageTable *p3Table = p4Table->walkToWithAlloc(
            p4Index, (paddr_t) nullptr, userAccessible);
        if (p3Table == nullptr) {
            return false;
        }
        p3addr =
            p4Table->entries[p4Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        PageTable *p2Table =
            p3Table->walkToWithAlloc(p3Index, p3addr, userAccessible);
        if (p2Table == nullptr) {
            if (PhysAllocator::decrementMapCount(p3addr)) {
                p4Table->entries[p4Index].addr = 0;
                PhysAllocator::freePage(p3addr);
            }
            return false;
        }
        p2addr =
            p3Table->entries[p3Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        PageTable *p1Table =
            p2Table->walkToWithAlloc(p2Index, p2addr, userAccessible);
        if (p1Table == nullptr) {
            if (PhysAllocator::decrementMapCount(p2addr)) {
                p3Table->entries[p3Index].addr = 0;
                PhysAllocator::freePage(p2addr);
            }
            if (PhysAllocator::decrementMapCount(p3addr)) {
                p4Table->entries[p4Index].addr = 0;
                PhysAllocator::freePage(p3addr);
            }
            return false;
        }
        p1addr =
            p2Table->entries[p2Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        PageTableEntry &entry = p1Table->entries[p1Index];
        if (!entry.present) {
            entry.addr = flags;
            entry.present = true;
            if (physAddr == 0) {
                entry.addr |= PhysAllocator::newPage();
                if ((entry.addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK)) == 0) {
                    if (PhysAllocator::decrementMapCount(p1addr)) {
                        p2Table->entries[p2Index].addr = 0;
                        PhysAllocator::freePage(p1addr);
                    }
                    if (PhysAllocator::decrementMapCount(p2addr)) {
                        p3Table->entries[p3Index].addr = 0;
                        PhysAllocator::freePage(p2addr);
                    }
                    if (PhysAllocator::decrementMapCount(p3addr)) {
                        p4Table->entries[p4Index].addr = 0;
                        PhysAllocator::freePage(p3addr);
                    }
                    return false;
                }
            } else {
                entry.addr |= physAddr;
                if (entry.managed) {
                    PhysAllocator::incrementRefCount(entry.addr);
                }
            }
            PhysAllocator::incrementMapCount(p1addr);
        } else {
            bool managed = entry.managed;
            entry.addr &= ~(1ULL << 63);
            entry.lowFlags = 0;
            entry.addr |= flags;
            entry.managed = managed;
        }
        vmbaseInvalidateCache(addr);
        return true;
    }

    void VirtualMemoryMapper::freePageAt(vaddr_t addr) {
        PageTable *p4Table = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        vind_t p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        PageTable *p3Table, *p2Table, *p1Table;
        paddr_t p3addr, p2addr, p1addr;
        p3Table = p4Table->walkTo(p4Index);
        p2Table = p3Table->walkTo(p3Index);
        p1Table = p2Table->walkTo(p2Index);
        if (p1Table == nullptr) {
            return;
        }
        paddr_t pageAddr =
            p1Table->entries[p1Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        if (p1Table->entries[p1Index].managed) {
            PhysAllocator::freePage(pageAddr);
        }
        p1Table->entries[p1Index].addr = 0;
        p1addr =
            p2Table->entries[p2Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        p2addr =
            p3Table->entries[p3Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        p3addr =
            p4Table->entries[p4Index].addr & (~PAGE_TABLE_ENTRY_FLAGS_MASK);
        if (!p2Table->entries[p2Index].managed) {
            return;
        }
        if (PhysAllocator::decrementMapCount(p1addr)) {
            PhysAllocator::freePage(p1addr);
            p2Table->entries[p2Index].addr = 0;
        } else {
            return;
        }
        if (!p3Table->entries[p3Index].managed) {
            return;
        }
        if (PhysAllocator::decrementMapCount(p2addr)) {
            PhysAllocator::freePage(p2addr);
            p3Table->entries[p3Index].addr = 0;
        } else {
            return;
        }
        // do not deallocate l3 kernel pages
        // they should be equal across all page tables
        // so kernel won't see change in its part of address space
        if (p4Index > 255) {
            if (!p4Table->entries[p4Index].managed) {
                return;
            }
            if (PhysAllocator::decrementMapCount(p3addr)) {
                PhysAllocator::freePage(p3addr);
                p4Table->entries[p4Index].addr = 0;
            }
        }
        vmbaseInvalidateCache(addr);
    }

    bool VirtualMemoryMapper::mapNewPages(vaddr_t start, vaddr_t end) {
        for (uint64_t addr = start; addr < end; addr += 4096) {
            if (!mapNewPageAt(addr, 0, DEFAULT_KERNEL_FLAGS)) {
                for (uint64_t addr2 = start; addr2 < addr; addr2 += 4096) {
                    freePageAt(addr2);
                }
                return false;
            }
        }
        return true;
    }

    bool VirtualMemoryMapper::mapPages(vaddr_t start, vaddr_t end,
                                       paddr_t physAddr, uint64_t flags) {
        for (uint64_t addr = start; addr < end; addr += 4096) {
            if (!mapNewPageAt(addr, addr - start + physAddr, flags)) {
                for (uint64_t addr2 = start; addr2 < addr; addr2 += 4096) {
                    freePageAt(addr2);
                }
                return false;
            }
        }
        return true;
    }

    void VirtualMemoryMapper::freePages(vaddr_t start, vaddr_t end) {
        for (uint64_t addr = start; addr < end; addr += 4096) {
            freePageAt(addr);
        }
    }

    bool VirtualMemoryMapper::isAvailable(vaddr_t addr, bool priveleged) {
        PageTable *p4Table = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        vind_t p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        return p4Table->walkToWithPrivelegeCheck(p4Index, priveleged)
                   ->walkToWithPrivelegeCheck(p3Index, priveleged)
                   ->walkToWithPrivelegeCheck(p2Index, priveleged)
                   ->walkToWithPrivelegeCheck(p1Index, priveleged) != nullptr;
    }

    bool VirtualMemoryMapper::areAvailable(vaddr_t start, vaddr_t end,
                                           bool priveleged) {
        for (vaddr_t addr = start; addr < end; addr += 4096) {
            if (!isAvailable(addr, priveleged)) {
                return false;
            }
        }
        return true;
    }
}; // namespace memory