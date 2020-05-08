#include <memory/tmpvalloc.hpp>

namespace memory {
    vaddr_t TempVirtualAllocator::m_pageEnd;
    vaddr_t TempVirtualAllocator::m_unalignedEnd;
    bool TempVirtualAllocator::m_initialized;

    void TempVirtualAllocator::init(vaddr_t initMappingEnd) {
        if (!TempPhysAllocator::isInitialized()) {
            panic("[TempVirtualAllocator] Dependency TempPhysAllocator is not "
                  "initialized");
        }
        m_pageEnd = initMappingEnd;
        m_unalignedEnd = initMappingEnd;
        m_initialized = true;
    }
    namespace {
        void mapNewPageFromTempAlloc(vaddr_t addr) {
            vind_t p4Index, p3Index, p2Index, p1Index;
            p4Index = getP4Index(addr);
            p3Index = getP3Index(addr);
            p2Index = getP2Index(addr);
            p1Index = getP1Index(addr);
            PageTable *root = (PageTable *)p4TableVirtualAddress;
            root = root->walkToWithTempAlloc(p4Index);
            root = root->walkToWithTempAlloc(p3Index);
            root = root->walkToWithTempAlloc(p2Index);
            PageTableEntry *entry = root->entries + p1Index;
            paddr_t physAddr = TempPhysAllocator::newFrame();
            entry->addr = physAddr;
            entry->writable = true;
            entry->present = true;
            entry->managed = false;
            vmbaseInvalidateCache((vaddr_t)addr);
        }
    } // namespace

    void *TempVirtualAllocator::valloc(uint64_t size) {
        void *result = (void *)m_unalignedEnd;
        uint64_t newUnalignedEnd = (uint64_t)m_unalignedEnd + size;
        uint64_t newPageEnd = alignUp(newUnalignedEnd, 4096);
        for (vaddr_t addr = m_pageEnd; addr < newPageEnd; addr += 4096) {
            mapNewPageFromTempAlloc(addr);
        }
        m_pageEnd = newPageEnd;
        m_unalignedEnd = newUnalignedEnd;
        return result;
    }

}; // namespace memory