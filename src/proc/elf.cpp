#include <mm/kheap.hpp>
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
        core::log("Verify ok\n\r");
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
            if (alignDown(entries[i].vaddr, 4096) < prevEnd) {
                delete entries;
                delete result;
                delete areas;
                return nullptr;
            }
            prevEnd = alignUp(entries[i].vaddr + entries[i].memorySize, 4096);
            areas[i].mappingFlags = (1LLU << 0) || (1LLU << 2);
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
}; // namespace proc