#ifndef __MEMORY_INFO_HPP_INCLUDED__
#define __MEMORY_INFO_HPP_INCLUDED__

#include <boot/multiboot.hpp>

#define KERNEL_MAPPING_BASE 0xffff800000000000ULL

namespace memory {

    struct MemoryMapEntry {
        Uint64 base;
        Uint64 limit;
        multiboot::MemoryMapEntryType type;
        Uint32 : 32;
    };

    class BootMemoryInfo {
        static bool initialized;

    public:
        static void init(Uint64 header);
        INLINE static bool isInitialized() { return initialized; }
        static Uint64 kernelBase;
        static Uint64 kernelLimit;
        static Uint64 multibootBase;
        static Uint64 multibootLimit;
        static Uint64 upperLimit;
        static Uint32 mmapEntriesCount;
        static MemoryMapEntry* mmapEntries;
    };

} // namespace memory

#endif