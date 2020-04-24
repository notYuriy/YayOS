#ifndef __MEMORY_INFO_HPP_INCLUDED__
#define __MEMORY_INFO_HPP_INCLUDED__

#include <boot/multiboot.hpp>

#define KERNEL_MAPPING_BASE 0xffff800000000000ULL

namespace memory {

    struct MemoryMapEntry {
        uint64_t base;
        uint64_t limit;
        multiboot::MemoryMapEntryType type;
        uint32_t : 32;
    };

    class BootMemoryInfo {
        static bool m_initialized;

    public:
        static void init(uint64_t header);
        INLINE static bool isInitialized() { return m_initialized; }
        static uint64_t kernelBase;
        static uint64_t kernelLimit;
        static uint64_t multibootBase;
        static uint64_t multibootLimit;
        static uint64_t upperLimit;
        static uint32_t mmapEntriesCount;
        static MemoryMapEntry *mmapEntries;
    };

} // namespace memory

#endif