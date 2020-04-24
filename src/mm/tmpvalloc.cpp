#include <mm/tmpvalloc.hpp>

namespace memory {
    VAddr TempVirtualAllocator::m_pageEnd;
    VAddr TempVirtualAllocator::m_unalignedEnd;
    bool TempVirtualAllocator::m_initialized;

    void TempVirtualAllocator::init(VAddr initMappingEnd) {
        if (!TempPhysAllocator::isInitialized()) {
            panic("[TempVirtualAllocator] Dependency TempPhysAllocator is not "
                  "initialized");
        }
        m_pageEnd = initMappingEnd;
        m_unalignedEnd = initMappingEnd;
        m_initialized = true;
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

    void *TempVirtualAllocator::valloc(uint64_t size) {
        void *result = (void *)m_unalignedEnd;
        uint64_t newUnalignedEnd = (uint64_t)m_unalignedEnd + size;
        uint64_t newPageEnd = alignUp(newUnalignedEnd, 4096);
        for (VAddr addr = m_pageEnd; addr < newPageEnd; addr += 4096) {
            mapNewPageFromTempAlloc(addr);
        }
        m_pageEnd = newPageEnd;
        m_unalignedEnd = newUnalignedEnd;
        return result;
    }

}; // namespace memory