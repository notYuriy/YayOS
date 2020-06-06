#include <core/cpprt.hpp>
#include <proc/elf.hpp>

namespace proc {

    const char *elfMagic = "\x7F"
                           "ELF";
    bool ElfHeader::verify() {
        return !((!streqn(magic, elfMagic, 4)) || fileClass != ELF_CLASS_64 ||
                 dataEncoding != ELF_DATA_LSB || ABI != ELF_SYSV_ABI ||
                 ABIVersion != ELF_SYSV_ABI_VERSION || type != ELF_EXEC ||
                 machine != ELF_ARCH_X86_64 || version != ELF_CUR_VERSION ||
                 headerSize != ELF_HEADER_SIZE ||
                 programHeaderOffset < (int64_t)headerSize ||
                 programEntrySize != sizeof(ElfProgramHeaderEntry));
    }

    Elf *parseElf(fs::IFile *file) {
        Elf *result = new Elf;
        if (result == nullptr) {
            return nullptr;
        }
        int64_t read =
            file->read(sizeof(ElfHeader), (uint8_t *)&(result->head));
        if (read != sizeof(ElfHeader)) {
            delete result;
            return nullptr;
        }
        if (!result->head.verify()) {
            delete result;
            return nullptr;
        }
        uint16_t entriesCount = result->head.programEntryCount;
        if (file->lseek(result->head.programHeaderOffset, fs::SEEK_SET) < 0) {
            delete result;
            return nullptr;
        }
        ElfProgramHeaderEntry *entries =
            new ElfProgramHeaderEntry[entriesCount];
        if (entries == nullptr) {
            delete result;
            return nullptr;
        }
        ElfMemoryArea *areas = new ElfMemoryArea[entriesCount];
        if (entries == nullptr) {
            delete result;
            delete entries;
            return nullptr;
        }
        int64_t toRead = entriesCount * sizeof(ElfMemoryArea);
        if (file->read(toRead, (uint8_t *)(entries)) != toRead) {
            delete entries;
            delete result;
            delete areas;
            return nullptr;
        }
        memory::vaddr_t prevEnd = 4096;
        for (uint16_t i = 0; i < entriesCount; ++i) {
            if (entries[i].type == ELF_NULL) {
                areas[i].isRequired = false;
                continue;
            }
            areas[i].isRequired = true;
            if (entries[i].type != ELF_LOAD ||
                alignDown(entries[i].vaddr, 4096) < prevEnd ||
                entries[i].fileSize < 0) {
                delete entries;
                delete result;
                delete areas;
                return nullptr;
            }
            prevEnd = alignUp(entries[i].vaddr + entries[i].memorySize, 4096);
            areas[i].mappingFlags = (1LLU << 0) | (1LLU << 2);
            if ((entries[i].flags & 2) == 2) {
                areas[i].mappingFlags |= (1LLU << 1);
            }
            if ((entries[i].flags & 1) != 1) {
                areas[i].mappingFlags |= (1LLU << 63);
            }
            areas[i].memoryOffset = (uint8_t *)entries[i].vaddr;
            areas[i].memoryBase = alignDown(entries[i].vaddr, 4096);
            areas[i].memoryLimit = prevEnd;
            areas[i].fileOffset = entries[i].offset;
            areas[i].fileSize = entries[i].fileSize;
        }
        result->areas = areas;
        result->areasCount = entriesCount;
        return result;
    }

    bool ElfMemoryArea::map(fs::IFile *file) {
        if (!isRequired) {
            return true;
        }
        bool result = memory::VirtualMemoryMapper::mapPages(
            memoryBase, memoryLimit, 0, memory::DEFAULT_KERNEL_FLAGS);
        if (!result) {
            return false;
        }
        int64_t iresult = file->lseek(fileOffset, fs::SEEK_SET);
        if (iresult == -1) {
            memory::VirtualMemoryMapper::freePages(memoryBase, memoryLimit);
            return false;
        }
        iresult = file->read(fileSize, memoryOffset);
        if (iresult != fileSize) {
            memory::VirtualMemoryMapper::freePages(memoryBase, memoryLimit);
            return false;
        }
        return memory::VirtualMemoryMapper::mapPages(memoryBase, memoryLimit, 0,
                                                     mappingFlags);
    }

    void ElfMemoryArea::unmap() {
        memory::VirtualMemoryMapper::freePages(memoryBase, memoryLimit);
    }

    bool Elf::load(fs::IFile *file, memory::UserVirtualAllocator *usralloc) {
        for (uint64_t i = 0; i < areasCount; ++i) {
            if (!usralloc->reserve(areas[i].memoryBase,
                                   areas[i].memoryLimit -
                                       areas[i].memoryBase) ||
                !areas[i].map(file)) {
                for (uint64_t j = 0; j < i; ++j) {
                    areas[i].unmap();
                }
                return false;
            } else {
            }
        }
        return true;
    }

}; // namespace proc