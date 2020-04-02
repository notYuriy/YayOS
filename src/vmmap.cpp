#include <vmmap.hpp>

namespace memory {
    void VirtualMemoryMapper::mapNewPageAt(VAddr addr, PAddr physAddr,
                                           bool managed = true) {
        PageTable* root = (PageTable*)p4TableVirtualAddress;
        VIndex p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        PAddr p3addr, p2addr, p1addr;
        p3addr = root->entries[p4Index].addr && (~pageTableEntryFlagsMask);
        PageTable* next = root->walkToWithAlloc(p4Index, (PAddr) nullptr);
        p2addr = next->entries[p3Index].addr && (~pageTableEntryFlagsMask);
        next = next->walkToWithAlloc(p3Index, p3addr);
        p1addr = next->entries[p2Index].addr && (~pageTableEntryFlagsMask);
        next = next->walkToWithAlloc(p2Index, p2addr);
        PageTableEntry& entry = next->entries[p1Index];
        if (!entry.present) {
            if (physAddr == 0) {
                entry.addr = PhysAllocator::newPage();
                entry.writable = true;
                entry.present = true;
            } else {
                entry.addr = physAddr;
                if (managed) {
                    PhysAllocator::incrementRefCount(entry.addr);
                }
            }
            entry.writable = true;
            entry.present = true;
            PhysAllocator::incrementMapCount(p1addr);
        }
    }

    void VirtualMemoryMapper::freePageAt(VAddr addr) {
        PageTable* p4Table = (PageTable*)p4TableVirtualAddress;
        VIndex p4Index = getP4Index(addr), p3Index = getP3Index(addr),
               p2Index = getP2Index(addr), p1Index = getP1Index(addr);
        PageTable *p3Table, *p2Table, *p1Table;
        PAddr p3addr, p2addr, p1addr;
        p3Table = p4Table->walkTo(p4Index);
        p2Table = p3Table->walkTo(p3Index);
        p1Table = p2Table->walkTo(p2Index);
        PAddr pageAddr =
            p1Table->entries[p1Index].addr & (~pageTableEntryFlagsMask);
        PhysAllocator::freePage(pageAddr);
        p1Table->entries[p1Index].addr = 0;
        p1addr = p2Table->entries[p2Index].addr & (~pageTableEntryFlagsMask);
        p2addr = p3Table->entries[p3Index].addr & (~pageTableEntryFlagsMask);
        p3addr = p4Table->entries[p4Index].addr & (~pageTableEntryFlagsMask);
        if (PhysAllocator::decrementMapCount(p1addr)) {
            PhysAllocator::freePage(p1addr);
            p2Table->entries[p2Index].addr = 0;
        }
        if (PhysAllocator::decrementMapCount(p2addr)) {
            PhysAllocator::freePage(p2addr);
            p3Table->entries[p3Index].addr = 0;
        }
        if (PhysAllocator::decrementMapCount(p3addr)) {
            PhysAllocator::freePage(p3addr);
            p4Table->entries[p4Index].addr = 0;
        }
    }

    void VirtualMemoryMapper::mapNewPages(VAddr start, VAddr end) {
        for (Uint64 addr = start; addr < end; addr += 4096) {
            mapNewPageAt(addr, 0);
        }
    }

    void VirtualMemoryMapper::mapPages(VAddr start, VAddr end, PAddr physAddr,
                                       bool managed) {
        for (Uint64 addr = start; addr < end; addr += 4096) {
            mapNewPageAt(addr, physAddr - start + addr, managed);
        }
    }

    void VirtualMemoryMapper::freePages(VAddr start, VAddr end) {
        for (Uint64 addr = start; addr < end; addr += 4096) {
            freePageAt(addr);
        }
    }
}; // namespace memorya