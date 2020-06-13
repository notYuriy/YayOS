#include <memory/cow.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/physalloc.hpp>
#include <memory/vmmap.hpp>
#include <proc/state.hpp>
#include <x86_64/interrupts.hpp>

namespace memory {
    constexpr uint64_t PAGE_FAULT_INDEX = 0xe;
    bool CoW::m_initialized;

    void handleSecurityViolation() {
        core::log("Security violation\n\r");
        proc::ProcessManager::exit();
    }

    void dump(uint64_t addr) {
        vind_t p4index, p3index, p2index, p1index;
        p4index = getP4Index(addr);
        p3index = getP3Index(addr);
        p2index = getP2Index(addr);
        p1index = getP1Index(addr);
        PageTable *p4Table = (PageTable *)(P4_TABLE_VIRTUAL_ADDRESS);
        PageTable *p3Table = p4Table->walkTo(p4index);
        PageTable *p2Table = p3Table->walkTo(p3index);
        PageTable *p1Table = p2Table->walkTo(p2index);
        core::log("CR2: %p\n\r", addr);
        core::log("P4: %p\n\r", getPageTable());
        core::log("P3: %p %d\n\r", p4Table->entries[p4index].getAddr(),
                  (int)(p4Table->entries[p4index].wasWritable));
        core::log("P2: %p %d\n\r", p3Table->entries[p3index].getAddr(),
                  (int)(p3Table->entries[p3index].wasWritable));
        core::log("P1: %p %d\n\r", p2Table->entries[p2index].getAddr(),
                  (int)(p2Table->entries[p2index].wasWritable));
        core::log("P0: %p %d\n\r", p1Table->entries[p1index].getAddr(),
                  (int)(p1Table->entries[p1index].wasWritable));
    }

    extern "C" void pageFaultHandler();
    PageTable *CoW::checkAndMoveToNext(PageTable *current, vind_t index,
                                       uint64_t errorCode) {
        PageTableEntry *entry = current->entries + index;
        bool fromUserspace = (errorCode & (1ULL << 2)) != 0;
        bool writableAccess = (errorCode & (1ULL << 1)) != 0;
        bool instructionFetch = (errorCode & (1ULL << 4)) != 0;
        if (!entry->present) {
            if (!fromUserspace) {
                asm __volatile__("cli" :::);
                panic("[Page Fault Handler] Attempt to access unmapped virtual "
                      "address in kernel code");
            }
            handleSecurityViolation();
        }
        if ((!entry->userAccessible) && fromUserspace) {
            proc::ProcessManager::exit();
        }
        if (instructionFetch && ((entry->addr & (1ULL << 63)) != 0)) {
            if (!fromUserspace) {
                panic("[Page Fault Handler] Attempt to execute instruction "
                      "located at no execute "
                      "page in kernel code");
            }
            handleSecurityViolation();
        }
        if (entry->cow) {
            return current->walkTo(index);
        } else if (writableAccess && !(entry->writable)) {
            if (!fromUserspace) {
                panic(
                    "[Page Fault Handler] Attempt to write at readonly mapped "
                    "address from cpl=0");
            }
            handleSecurityViolation();
        }
        return current->walkTo(index);
    }
    extern "C" void
    pageFaultHandlerWithFrame(UNUSED proc::SchedulerIntFrame *frame) {
        core::log("FAULT at %p!\n\r", frame->rip);
        while (1)
            ;
        vaddr_t addr = getCR2();
        vind_t p4Index, p3Index, p2Index, p1Index;
        p4Index = getP4Index(addr);
        p3Index = getP3Index(addr);
        p2Index = getP2Index(addr);
        p1Index = getP1Index(addr);
        paddr_t p3NewPaddr = 0, p2NewPaddr = 0, p1NewPaddr = 0, p0NewPaddr = 0;
        paddr_t p3OldPaddr, p2OldPaddr, p1OldPaddr, p0OldPaddr;
        PageTable *p4Table, *p3Table, *p2Table, *p1Table, *p0Table;
        p4Table = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        p3Table = CoW::checkAndMoveToNext(p4Table, p4Index, frame->errcode);
        p2Table = CoW::checkAndMoveToNext(p3Table, p3Index, frame->errcode);
        p1Table = CoW::checkAndMoveToNext(p2Table, p2Index, frame->errcode);
        p0Table = (PageTable *)toCanonical(
            (vaddr_t)CoW::checkAndMoveToNext(p1Table, p1Index, frame->errcode));
        p3OldPaddr = p4Table->entries[p4Index].getAddr();
        p2OldPaddr = p3Table->entries[p3Index].getAddr();
        p1OldPaddr = p2Table->entries[p2Index].getAddr();
        p0OldPaddr = p1Table->entries[p1Index].getAddr();
        p0NewPaddr = p1NewPaddr = p2NewPaddr = p3NewPaddr = 0;
        if (p1Table->entries[p1Index].cow) {
            p0NewPaddr = PhysAllocator::newPage();
            if (p0NewPaddr == 0) {
                goto allocFailureCleanup;
            }
            vaddr_t newMapping = KernelVirtualAllocator::getMapping(
                0x1000, p0NewPaddr, DEFAULT_UNMANAGED_FLAGS);
            if (newMapping == 0) {
                goto allocFailureCleanup;
            }
            memcpy((void *)newMapping, p0Table, 0x1000);
            memory::KernelVirtualAllocator::unmapAt(newMapping, 0x1000);
        } else {
            handleSecurityViolation();
        }
        if (p2Table->entries[p2Index].cow) {
            p1NewPaddr = PhysAllocator::newPage();
            if (p1NewPaddr == 0) {
                goto allocFailureCleanup;
            }
            vaddr_t newMapping = KernelVirtualAllocator::getMapping(
                0x1000, p1NewPaddr, DEFAULT_UNMANAGED_FLAGS);
            if (newMapping == 0) {
                goto allocFailureCleanup;
            }
            memcpy((void *)newMapping, p1Table, 0x1000);
            PageTable *table = (PageTable *)newMapping;
            table->entries[p1Index].setAddr(p0NewPaddr);
            table->entries[p1Index].cow = false;
            table->entries[p1Index].writable =
                table->entries[p1Index].wasWritable;
            memory::KernelVirtualAllocator::unmapAt(newMapping, 0x1000);
        } else {
            p1Table->entries[p1Index].setAddr(p0NewPaddr);
            p1Table->entries[p1Index].cow = false;
            p1Table->entries[p1Index].writable =
                p1Table->entries[p1Index].wasWritable;
            vmbaseInvalidateCache((memory::vaddr_t)p1Table);
            vmbaseInvalidateCache(addr);
            PhysAllocator::freePage(p0OldPaddr);
            return;
        }
        if (p3Table->entries[p3Index].cow) {
            p2NewPaddr = PhysAllocator::newPage();
            if (p2NewPaddr == 0) {
                goto allocFailureCleanup;
            }
            vaddr_t newMapping = KernelVirtualAllocator::getMapping(
                0x1000, p2NewPaddr, DEFAULT_UNMANAGED_FLAGS);
            if (newMapping == 0) {
                goto allocFailureCleanup;
            }
            memcpy((void *)newMapping, p2Table, 0x1000);
            PageTable *table = (PageTable *)newMapping;
            table->entries[p2Index].setAddr(p1NewPaddr);
            table->entries[p2Index].cow = false;
            table->entries[p2Index].writable =
                table->entries[p2Index].wasWritable;
            memory::KernelVirtualAllocator::unmapAt(newMapping, 0x1000);
        } else {
            p2Table->entries[p2Index].setAddr(p1NewPaddr);
            p2Table->entries[p2Index].cow = false;
            p2Table->entries[p2Index].writable =
                p2Table->entries[p2Index].wasWritable;
            vmbaseInvalidateCache((memory::vaddr_t)p2Table);
            vmbaseInvalidateCache((memory::vaddr_t)p1Table);
            vmbaseInvalidateCache(addr);
            PhysAllocator::freePage(p1OldPaddr);
            PhysAllocator::freePage(p0OldPaddr);
            return;
        }
        if (p4Table->entries[p4Index].cow) {
            p3NewPaddr = PhysAllocator::newPage();
            if (p3NewPaddr == 0) {
                goto allocFailureCleanup;
            }
            vaddr_t newMapping = KernelVirtualAllocator::getMapping(
                0x1000, p3NewPaddr, DEFAULT_UNMANAGED_FLAGS);
            if (newMapping == 0) {
                goto allocFailureCleanup;
            }
            memcpy((void *)newMapping, p3Table, 0x1000);
            PageTable *table = (PageTable *)newMapping;
            table->entries[p3Index].setAddr(p2NewPaddr);
            table->entries[p3Index].cow = false;
            table->entries[p3Index].writable =
                table->entries[p3Index].wasWritable;
            memory::KernelVirtualAllocator::unmapAt(newMapping, 0x1000);
        } else {
            p3Table->entries[p3Index].setAddr(p2NewPaddr);
            p3Table->entries[p3Index].cow = false;
            p3Table->entries[p3Index].writable =
                p3Table->entries[p3Index].wasWritable;
            vmbaseInvalidateCache((memory::vaddr_t)p3Table);
            vmbaseInvalidateCache((memory::vaddr_t)p2Table);
            vmbaseInvalidateCache((memory::vaddr_t)p1Table);
            vmbaseInvalidateCache(addr);
            PhysAllocator::freePage(p2OldPaddr);
            PhysAllocator::freePage(p1OldPaddr);
            PhysAllocator::freePage(p0OldPaddr);
            return;
        }
        p4Table->entries[p4Index].setAddr(p3NewPaddr);
        p4Table->entries[p4Index].cow = false;
        p4Table->entries[p4Index].writable =
            p4Table->entries[p4Index].wasWritable;
        vmbaseInvalidateCache((memory::vaddr_t)p4Table);
        vmbaseInvalidateCache((memory::vaddr_t)p3Table);
        vmbaseInvalidateCache((memory::vaddr_t)p2Table);
        vmbaseInvalidateCache((memory::vaddr_t)p1Table);
        vmbaseInvalidateCache(addr);
        PhysAllocator::freePage(p3OldPaddr);
        PhysAllocator::freePage(p2OldPaddr);
        PhysAllocator::freePage(p1OldPaddr);
        PhysAllocator::freePage(p0OldPaddr);
        return;
    allocFailureCleanup:
        if (p3NewPaddr != 0) {
            PhysAllocator::freePage(p3NewPaddr);
        }
        if (p2NewPaddr != 0) {
            PhysAllocator::freePage(p2NewPaddr);
        }
        if (p1NewPaddr != 0) {
            PhysAllocator::freePage(p1NewPaddr);
        }
        if (p0NewPaddr != 0) {
            PhysAllocator::freePage(p0NewPaddr);
        }
    }
    void CoW::init() {
        x86_64::IDT::install(PAGE_FAULT_INDEX,
                             (x86_64::IDTVector)pageFaultHandler, 0, false);
        m_initialized = true;
    }
    void CoW::markAsCoW() {
        PageTable *table = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        for (uint64_t i = 0; i < 256; ++i) {
            if (table->entries[i].present) {
                if (!(table->entries[i].cow)) {
                    table->entries[i].wasWritable = table->entries[i].writable;
                    table->entries[i].writable = false;
                    table->entries[i].cow = true;
                }
                PhysAllocator::incrementRefCount(table->entries[i].getAddr());
                markAsCoWRec(3, table->walkTo(i));
            }
        }
    }
    void CoW::markAsCoWRec(uint64_t level, PageTable *table) {
        if (level == 0) {
            return;
        }
        for (uint64_t i = 0; i < 512; ++i) {
            if (table->entries[i].present) {
                if (!(table->entries[i].cow)) {
                    table->entries[i].wasWritable = table->entries[i].writable;
                    table->entries[i].writable = false;
                    table->entries[i].cow = true;
                }
                PhysAllocator::incrementRefCount(table->entries[i].getAddr());
                markAsCoWRec(level - 1, table->walkTo(i));
            }
        }
    }
    uint64_t CoW::clonePageTable() {
        paddr_t newFrame = PhysAllocator::newPage();
        if (newFrame == 0) {
            return 0;
        }
        vaddr_t mapping = memory::KernelVirtualAllocator::getMapping(
            0x1000, newFrame, DEFAULT_UNMANAGED_FLAGS);
        if (mapping == 0) {
            PhysAllocator::freePage(newFrame);
            return 0;
        }
        markAsCoW();
        memcpy((void *)mapping, (void *)P4_TABLE_VIRTUAL_ADDRESS, 0x1000);
        PageTable *table = (PageTable *)mapping;
        table->entries[511].setAddr(newFrame);
        memory::KernelVirtualAllocator::unmapAt(mapping, 0x1000);
        return newFrame;
    }
    uint64_t CoW::newPageTable() {
        paddr_t newFrame = PhysAllocator::newPage();
        vaddr_t mapping = memory::KernelVirtualAllocator::getMapping(
            0x1000, newFrame, DEFAULT_UNMANAGED_FLAGS);
        memcpy((void *)mapping, (void *)P4_TABLE_VIRTUAL_ADDRESS, 0x1000);
        PageTable *table = (PageTable *)mapping;
        for (int i = 0; i < 256; ++i) {
            table->entries[i].addr = 0;
        }
        table->entries[511].setAddr(newFrame);
        memory::KernelVirtualAllocator::unmapAt(mapping, 0x1000);
        return newFrame;
    }
    void CoW::deallocateUserMemory() {
        PageTable *addr = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        for (uint64_t i = 0; i < 256; ++i) {
            if (addr->entries[i].present) {
                disposePageTableRec(3, addr->walkTo(i));
                PhysAllocator::freePage(addr->entries[i].getAddr());
                addr->entries[i].addr = 0;
            }
        }
        vmbaseInvalidateAll();
    }
    void CoW::disposePageTableRec(uint64_t level, PageTable *addr) {
        if (level == 0) {
            return;
        }
        for (uint64_t i = 0; i < 512; ++i) {
            if (addr->entries[i].present) {
                disposePageTableRec(level - 1, addr->walkTo(i));
                PhysAllocator::freePage(addr->entries[i].getAddr());
            }
        }
    }
}; // namespace memory