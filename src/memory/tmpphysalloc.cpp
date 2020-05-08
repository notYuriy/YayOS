#include <memory/tmpphysalloc.hpp>

namespace memory {
    memory::MemoryMapEntry *TempPhysAllocator::m_currentEntry;
    uint64_t TempPhysAllocator::m_areaUsed;
    paddr_t TempPhysAllocator::m_currentPhysAddr;
    bool TempPhysAllocator::m_initialized;

    bool TempPhysAllocator::afterCurrentMemoryArea() {
        return m_currentEntry->limit + 4096 < m_currentPhysAddr;
    }

    bool TempPhysAllocator::beforeCurrentMemoryArea() {
        return m_currentEntry->base > m_currentPhysAddr;
    }

    void TempPhysAllocator::AdjustMemoryArea() {
        while (true) {
            if (!afterCurrentMemoryArea()) {
                if (beforeCurrentMemoryArea()) {
                    m_currentPhysAddr = alignUp(m_currentEntry->base, 4096);
                }
                if (m_currentEntry->type ==
                    multiboot::MemoryMapEntryType::Available) {
                    break;
                }
            }
            m_currentEntry++;
            m_areaUsed++;
            if (m_areaUsed >= memory::BootMemoryInfo::mmapEntriesCount) {
                panic(
                    "[TempPhysAllocator] Can't find any available memory area");
            }
        }
    }

    bool TempPhysAllocator::CheckMultibootOverlap() {
        if (m_currentPhysAddr >= memory::BootMemoryInfo::multibootBase &&
            (m_currentPhysAddr < memory::BootMemoryInfo::multibootLimit)) {
            m_currentPhysAddr = memory::BootMemoryInfo::multibootLimit;
            return true;
        }
        return false;
    }

    bool TempPhysAllocator::CheckInitrdOverlap() {
        if (m_currentPhysAddr >= memory::BootMemoryInfo::initrdBase &&
            (m_currentPhysAddr < memory::BootMemoryInfo::initrdLimit)) {
            m_currentPhysAddr = memory::BootMemoryInfo::initrdLimit;
            return true;
        }
        return false;
    }

    void TempPhysAllocator::init() {
        if (!memory::BootMemoryInfo::isInitialized()) {
            panic("[TempPhysAllocator] BootMemoryInfo is not initialized\n\r");
        }
        m_currentPhysAddr = memory::BootMemoryInfo::kernelLimit;
        m_currentEntry = memory::BootMemoryInfo::mmapEntries;
        if (memory::BootMemoryInfo::mmapEntriesCount < 1) {
            panic("[TempPhysAllocator] No memory areas found\n\r");
        }
        m_areaUsed = 0;
        AdjustMemoryArea();
        CheckMultibootOverlap();
        CheckInitrdOverlap();
        AdjustMemoryArea();
        CheckMultibootOverlap();
        CheckInitrdOverlap();
        AdjustMemoryArea();
        CheckMultibootOverlap();
        CheckInitrdOverlap();
        m_initialized = true;
    }

    paddr_t TempPhysAllocator::getFirstUnusedFrame() {
        return m_currentPhysAddr;
    }

    paddr_t TempPhysAllocator::newFrame() {
        AdjustMemoryArea();
        CheckMultibootOverlap();
        CheckInitrdOverlap();
        AdjustMemoryArea();
        CheckMultibootOverlap();
        CheckInitrdOverlap();
        AdjustMemoryArea();
        CheckMultibootOverlap();
        CheckInitrdOverlap();
        paddr_t result = m_currentPhysAddr;
        m_currentPhysAddr += 4096;
        return result;
    }

} // namespace memory