#include <memory/memoryinfo.hpp>

namespace memory {
    bool BootMemoryInfo::m_initialized;
    uint64_t BootMemoryInfo::kernelBase;
    uint64_t BootMemoryInfo::kernelLimit;
    uint64_t BootMemoryInfo::multibootBase;
    uint64_t BootMemoryInfo::multibootLimit;
    uint64_t BootMemoryInfo::upperLimit;
    uint32_t BootMemoryInfo::mmapEntriesCount;
    uint64_t BootMemoryInfo::initrdBase;
    uint64_t BootMemoryInfo::initrdLimit;
    uint64_t BootMemoryInfo::initrdStart;
    MemoryMapEntry *BootMemoryInfo::mmapEntries;

    void BootMemoryInfo::init(uint64_t physHeader) {
        upperLimit = 0;
        multiboot::BootInfoHeader *header =
            (multiboot::BootInfoHeader *)(physHeader + KERNEL_MAPPING_BASE);
        BootMemoryInfo::multibootBase = alignDown(physHeader, 4096);
        BootMemoryInfo::multibootLimit =
            alignUp(physHeader + (uint64_t)(header->totalSize), 4096);
        multiboot::BootInfoTag *tag = header->firstTag;
        multiboot::MemoryMapTag *memoryMapTag = nullptr;
        multiboot::ElfSectionsTag *elfSectionsTag = nullptr;
        multiboot::ModuleTag *initrd = nullptr;
        while (!tag->isTerminator()) {
            if (tag->type == multiboot::BootInfoTagType::MemoryMap) {
                memoryMapTag = tag->as<multiboot::MemoryMapTag>();
            }
            if (tag->type == multiboot::BootInfoTagType::ElfSections) {
                elfSectionsTag = tag->as<multiboot::ElfSectionsTag>();
            }
            if (tag->type == multiboot::BootInfoTagType::Module) {
                initrd = tag->as<multiboot::ModuleTag>();
            }
            tag = tag->next();
        }

        if (memoryMapTag != nullptr) {
            MemoryMapEntry *entries =
                (memory::MemoryMapEntry *)memoryMapTag->map;
            BootMemoryInfo::mmapEntries = entries;
            BootMemoryInfo::mmapEntriesCount = memoryMapTag->getEntriesCount();
            for (uint64_t i = 0; i < BootMemoryInfo::mmapEntriesCount; ++i) {
                uint64_t start = memoryMapTag->map[i].baseAddr;
                uint64_t end = start + memoryMapTag->map[i].length;
                entries[i].base = start;
                entries[i].limit = end;
                if (end > upperLimit) {
                    upperLimit = end;
                }
            }
        }

        upperLimit = alignDown(upperLimit - 1, 4096);

        uint64_t physKernelBase = (uint64_t)-1;
        uint64_t physKernelLimit = 0;

        if (elfSectionsTag != nullptr) {
            for (uint64_t i = 0; i < elfSectionsTag->getEntriesCount(); ++i) {
                multiboot::ElfSectionHeader &header =
                    elfSectionsTag->headers[i];
                uint64_t physBase = header.addr;
                if (physBase == 0) {
                    continue;
                }
                if (physBase > KERNEL_MAPPING_BASE) {
                    physBase -= KERNEL_MAPPING_BASE;
                }
                uint64_t physLimit = physBase + header.size;
                if (physKernelBase > physBase) {
                    physKernelBase = physBase;
                }
                if (physKernelLimit < physLimit) {
                    physKernelLimit = physLimit;
                }
            }
        }

        if (initrd != nullptr) {
            initrdBase = alignDown(initrd->moduleStart, 4096);
            initrdLimit = alignUp(initrd->moduleEnd, 4096);
            initrdStart = initrd->moduleStart;
        } else {
            initrdBase = 0;
            initrdLimit = 0;
        }

        BootMemoryInfo::kernelBase = alignDown(physKernelBase, 4096);
        BootMemoryInfo::kernelLimit = alignUp(physKernelLimit, 4096);
        BootMemoryInfo::m_initialized = true;
    }

} // namespace memory