#include <mm/physalloc.hpp>
#include <mm/tmpphysalloc.hpp>
#include <mm/vmbase.hpp>

namespace memory {

    PageTable *PageTable::walkToWithTempAlloc(VIndex index) {
        if (!entries[index].present) {
            entries[index].addr = TempPhysAllocator::newFrame();
            entries[index].lowFlags = 0;
            entries[index].writable = true;
            entries[index].present = true;
            entries[index].managed = false;
            zeroPage(walkTo(index));
            vmbaseInvalidateCache((memory::vaddr_t)this);
        }
        return walkTo(index);
    }

    PageTable *PageTable::walkToWithAlloc(VIndex index, paddr_t currentAddr) {
        if (!entries[index].present) {
            paddr_t newAddr = PhysAllocator::newPage();
            if (newAddr == 0) {
                return nullptr;
            }
            entries[index].addr = newAddr;
            if (currentAddr != (paddr_t) nullptr) {
                PhysAllocator::incrementMapCount(currentAddr);
            }
            entries[index].lowFlags = 0;
            entries[index].writable = true;
            entries[index].present = true;
            entries[index].managed = true;
            zeroPage(walkTo(index));
            vmbaseInvalidateCache((memory::vaddr_t)this);
        }
        return walkTo(index);
    }

} // namespace memory