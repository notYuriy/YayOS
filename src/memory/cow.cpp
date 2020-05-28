#include <memory/cow.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/physalloc.hpp>
#include <memory/vmmap.hpp>
#include <proc/state.hpp>
#include <x86_64/interrupts.hpp>

namespace memory {
    constexpr uint64_t PAGE_FAULT_INDEX = 0xe;
    bool CoW::m_initialized;
    extern "C" void pageFaultHandler();
    extern "C" void pageFaultHandlerWithFrame(proc::SchedulerIntFrame *frame);
    void CoW::init() {
        x86_64::IDT::install(PAGE_FAULT_INDEX,
                             (x86_64::IDTVector)pageFaultHandler);
        m_initialized = true;
    }
    void CoW::cowPresent(PageTable *table) {
        for (uint64_t i = 0; i < 256; ++i) {
            if (table->entries[i].present) {
                table->entries[i].writable = false;
                table->entries[i].cow = true;
            }
        }
    }
    uint64_t CoW::clonePageTable() {
        paddr_t newFrame = PhysAllocator::newPage();
        vaddr_t mapping = memory::KernelVirtualAllocator::getMapping(
            0x1000, newFrame, DEFAULT_UNMANAGED_FLAGS);
        memcpy((void *)mapping, (void *)P4_TABLE_VIRTUAL_ADDRESS, 0x1000);
        cowPresent((PageTable *)P4_TABLE_VIRTUAL_ADDRESS);
        cowPresent((PageTable *)mapping);
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
        memory::KernelVirtualAllocator::unmapAt(mapping, 0x1000);
        return newFrame;
    }
    void CoW::deallocateUserMemory() {
        PageTable *addr = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        for (uint64_t i = 0; i < 256; ++i) {
            if (addr->entries[i].present) {
                disposePageTableRec(3, addr->walkTo(i));
                PhysAllocator::freePage(addr->entries[i].addr &
                                        (~pageTableEntryFlagsMask));
            }
        }
    }
    void CoW::disposePageTableRec(uint64_t level, PageTable *addr) {
        if (level == 0) {
            return;
        }
        for (uint64_t i = 0; i < 512; ++i) {
            if (addr->entries[i].present) {
                disposePageTableRec(level - 1, addr->walkTo(i));
                PhysAllocator::freePage(addr->entries[i].addr &
                                        (~pageTableEntryFlagsMask));
            }
        }
    }
}; // namespace memory