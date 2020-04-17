#include <mm/tmpvalloc.hpp>

namespace memory {
    VAddr TempVirtualAllocator::pageEnd;
    VAddr TempVirtualAllocator::unalignedEnd;
    bool TempVirtualAllocator::initialized;

    void TempVirtualAllocator::init(VAddr initMappingEnd) {
        if (!TempPhysAllocator::isInitialized()) {
            panic("[TempVirtualAllocator] Dependency TempPhysAllocator is not "
                  "initialized");
        }
        pageEnd = initMappingEnd;
        unalignedEnd = initMappingEnd;
        initialized = true;
    }
    namespace {
        void mapNewPageFromTempAlloc(VAddr addr) {
            VIndex p4Index, p3Index, p2Index, p1Index;
            p4Index = getP4Index(addr);
            p3Index = getP3Index(addr);
            p2Index = getP2Index(addr);
            p1Index = getP1Index(addr);
            PageTable *root = (PageTable *)p4TableVirtualAddress;
            root = root->walkToWithTempAlloc(p4Index);
            root = root->walkToWithTempAlloc(p3Index);
            root = root->walkToWithTempAlloc(p2Index);
            PageTableEntry *entry = root->entries + p1Index;
            PAddr physAddr = TempPhysAllocator::newFrame();
            entry->addr = physAddr;
            entry->writable = true;
            entry->present = true;
            entry->managed = false;
            vmbaseInvalidateCache((VAddr)addr);
        }
    } // namespace

    void *TempVirtualAllocator::valloc(Uint64 size) {
        void *result = (void *)unalignedEnd;
        Uint64 newUnalignedEnd = (Uint64)unalignedEnd + size;
        Uint64 newPageEnd = alignUp(newUnalignedEnd, 4096);
        for (VAddr addr = pageEnd; addr < newPageEnd; addr += 4096) {
            mapNewPageFromTempAlloc(addr);
        }
        pageEnd = newPageEnd;
        unalignedEnd = newUnalignedEnd;
        return result;
    }

}; // namespace memory