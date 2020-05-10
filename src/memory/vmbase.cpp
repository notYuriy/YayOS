#include <memory/physalloc.hpp>
#include <memory/tmpphysalloc.hpp>
#include <memory/vmbase.hpp>

namespace memory {

    PageTable *PageTable::walkToWithTempAlloc(vind_t index) {
        if (this == nullptr) {
            return nullptr;
        }
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

    PageTable *PageTable::walkToWithAlloc(vind_t index, paddr_t currentAddr,
                                          bool userAccessible) {
        if (this == nullptr) {
            return nullptr;
        }
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
            entries[index].userAccessible = userAccessible;
            zeroPage(walkTo(index));
            vmbaseInvalidateCache((memory::vaddr_t)this);
        } else {
            if (userAccessible) {
                entries[index].userAccessible = true;
                vmbaseInvalidateCache((memory::vaddr_t)this);
            }
        }
        return walkTo(index);
    }

} // namespace memory