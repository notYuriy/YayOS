#include <mm/vmmap.hpp>

namespace memory {
    bool VirtualMemoryMapper::mapNewPageAt(vaddr_t addr, paddr_t physAddr,
                                           uint64_t flags) {
        PageTable *p4Table = (PageTable *)p4TableVirtualAddress;
        VIndex p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        paddr_t p3addr, p2addr, p1addr;
        p3addr = p4Table->entries[p4Index].addr && (~pageTableEntryFlagsMask);
        PageTable *p3Table =
            p4Table->walkToWithAlloc(p4Index, (paddr_t) nullptr);
        if (p3Table == nullptr) {
            return false;
        }
        p2addr = p3Table->entries[p3Index].addr && (~pageTableEntryFlagsMask);
        PageTable *p2Table = p3Table->walkToWithAlloc(p3Index, p3addr);
        if (p2Table == nullptr) {
            if (PhysAllocator::decrementMapCount(p3addr)) {
                p4Table->entries[p4Index].addr = 0;
                PhysAllocator::freePage(p3addr);
            }
            return false;
        }
        p1addr = p2Table->entries[p2Index].addr && (~pageTableEntryFlagsMask);
        PageTable *p1Table = p2Table->walkToWithAlloc(p2Index, p2addr);
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
        PageTableEntry &entry = p1Table->entries[p1Index];
        if (!entry.present) {
            entry.addr = flags;
            entry.present = true;
            if (physAddr == 0) {
                entry.addr |= PhysAllocator::newPage();
                if (entry.addr && (~pageTableEntryFlagsMask) == 0) {
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
        }
        vmbaseInvalidateCache(addr);
        return true;
    }

    bool VirtualMemoryMapper::freePageAt(vaddr_t addr) {
        PageTable *p4Table = (PageTable *)p4TableVirtualAddress;
        VIndex p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        PageTable *p3Table, *p2Table, *p1Table;
        paddr_t p3addr, p2addr, p1addr;
        p3Table = p4Table->walkTo(p4Index);
        p2Table = p3Table->walkTo(p3Index);
        p1Table = p2Table->walkTo(p2Index);
        paddr_t pageAddr =
            p1Table->entries[p1Index].addr & (~pageTableEntryFlagsMask);
        if (p1Table->entries[p1Index].managed) {
            PhysAllocator::freePage(pageAddr);
        }
        p1Table->entries[p1Index].addr = 0;
        p1addr = p2Table->entries[p2Index].addr & (~pageTableEntryFlagsMask);
        p2addr = p3Table->entries[p3Index].addr & (~pageTableEntryFlagsMask);
        p3addr = p4Table->entries[p4Index].addr & (~pageTableEntryFlagsMask);
        if (!p2Table->entries[p2Index].managed) {
            return true;
        }
        if (PhysAllocator::decrementMapCount(p1addr)) {
            PhysAllocator::freePage(p1addr);
            p2Table->entries[p2Index].addr = 0;
        } else {
            return true;
        }
        if (!p3Table->entries[p3Index].managed) {
            return true;
        }
        if (PhysAllocator::decrementMapCount(p2addr)) {
            PhysAllocator::freePage(p2addr);
            p3Table->entries[p3Index].addr = 0;
        } else {
            return true;
        }
        if (!p4Table->entries[p4Index].managed) {
            return true;
        }
        if (PhysAllocator::decrementMapCount(p3addr)) {
            PhysAllocator::freePage(p3addr);
            p4Table->entries[p4Index].addr = 0;
        }
        vmbaseInvalidateCache(addr);
        return true;
    }

    bool VirtualMemoryMapper::mapNewPages(vaddr_t start, vaddr_t end) {
        for (uint64_t addr = start; addr < end; addr += 4096) {
            if (!mapNewPageAt(addr, 0, defaultKernelFlags)) {
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

    bool VirtualMemoryMapper::freePages(vaddr_t start, vaddr_t end) {
        for (uint64_t addr = start; addr < end; addr += 4096) {
            freePageAt(addr);
        }
        return true;
    }
}; // namespace memory