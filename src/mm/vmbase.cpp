#include <physalloc.hpp>
#include <tmpphysalloc.hpp>
#include <vmbase.hpp>

namespace memory {

    PageTable* PageTable::walkToWithTempAlloc(VIndex index) {
        if (!entries[index].present) {
            entries[index].addr = TempPhysAllocator::newFrame();
            entries[index].lowFlags = 0;
            entries[index].writable = true;
            entries[index].present = true;
            zeroPage(walkTo(index));
            vmbaseInvalidateCache((memory::VAddr)this);
        }
        return walkTo(index);
    }

    PageTable* PageTable::walkToWithAlloc(VIndex index, PAddr currentAddr) {
        if (!entries[index].present) {
            entries[index].addr = PhysAllocator::newPage();
            if (currentAddr != (PAddr) nullptr) {
                PhysAllocator::incrementMapCount(currentAddr);
            }
            entries[index].lowFlags = 0;
            entries[index].writable = true;
            entries[index].present = true;
            zeroPage(walkTo(index));
            vmbaseInvalidateCache((memory::VAddr)this);
        }
        return walkTo(index);
    }

} // namespace memory