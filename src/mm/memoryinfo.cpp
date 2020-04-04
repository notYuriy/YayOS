#include <memoryinfo.hpp>

namespace memory {
    bool BootMemoryInfo::initialized;
    Uint64 BootMemoryInfo::kernelBase;
    Uint64 BootMemoryInfo::kernelLimit;
    Uint64 BootMemoryInfo::multibootBase;
    Uint64 BootMemoryInfo::multibootLimit;
    Uint64 BootMemoryInfo::upperLimit;
    Uint32 BootMemoryInfo::mmapEntriesCount;
    MemoryMapEntry* BootMemoryInfo::mmapEntries;

    void BootMemoryInfo::init(Uint64 physHeader) {
        upperLimit = 0;
        multiboot::BootInfoHeader* header =
            (multiboot::BootInfoHeader*)(physHeader + KERNEL_MAPPING_BASE);
        BootMemoryInfo::multibootBase = alignDown(physHeader, 4096);
        BootMemoryInfo::multibootLimit =
            alignUp(physHeader + (Uint64)(header->totalSize), 4096);
        multiboot::BootInfoTag* tag = header->firstTag;
        multiboot::MemoryMapTag* memoryMapTag = nullptr;
        multiboot::ElfSectionsTag* elfSectionsTag = nullptr;
        while (!tag->isTerminator()) {
            if (tag->type == multiboot::BootInfoTagType::MemoryMap) {
                memoryMapTag = tag->as<multiboot::MemoryMapTag>();
            }
            if (tag->type == multiboot::BootInfoTagType::ElfSections) {
                elfSectionsTag = tag->as<multiboot::ElfSectionsTag>();
            }
            tag = tag->next();
        }

        if (memoryMapTag != nullptr) {
            MemoryMapEntry* entries =
                (memory::MemoryMapEntry*)memoryMapTag->map;
            BootMemoryInfo::mmapEntries = entries;
            BootMemoryInfo::mmapEntriesCount = memoryMapTag->getEntriesCount();
            for (Uint64 i = 0; i < BootMemoryInfo::mmapEntriesCount; ++i) {
                Uint64 start = memoryMapTag->map[i].baseAddr;
                Uint64 end = start + memoryMapTag->map[i].length;
                entries[i].base = start;
                entries[i].limit = end;
                if (end > upperLimit) {
                    upperLimit = end;
                }
            }
        }

        upperLimit = alignDown(upperLimit - 1, 4096);

        Uint64 physKernelBase = (Uint64)-1;
        Uint64 physKernelLimit = 0;

        if (elfSectionsTag != nullptr) {
            for (Uint64 i = 0; i < elfSectionsTag->getEntriesCount(); ++i) {
                multiboot::ElfSectionHeader& header =
                    elfSectionsTag->headers[i];
                Uint64 physBase = header.addr;
                if (physBase == 0) {
                    continue;
                }
                if (physBase > KERNEL_MAPPING_BASE) {
                    physBase -= KERNEL_MAPPING_BASE;
                }
                Uint64 physLimit = physBase + header.size;
                if (physKernelBase > physBase) {
                    physKernelBase = physBase;
                }
                if (physKernelLimit < physLimit) {
                    physKernelLimit = physLimit;
                }
            }
        }

        BootMemoryInfo::kernelBase = alignDown(physKernelBase, 4096);
        BootMemoryInfo::kernelLimit = alignUp(physKernelLimit, 4096);
        BootMemoryInfo::initialized = true;
    }

} // namespace memory