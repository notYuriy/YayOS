#include <core/uniqueptr.hpp>
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

    core::UniquePtr<Elf> parseElf(fs::IFile *file) {
        ElfHeader head;
        int64_t read = file->read(sizeof(ElfHeader), (uint8_t *)&(head));
        if (read != sizeof(ElfHeader)) {
            return core::UniquePtr<Elf>(nullptr);
        }
        if (!head.verify()) {
            return core::UniquePtr<Elf>(nullptr);
        }
        uint16_t entriesCount = head.programEntryCount;
        if (file->lseek(head.programHeaderOffset, fs::SEEK_SET) < 0) {
            return core::UniquePtr<Elf>(nullptr);
        }
        core::UniquePtr<ElfProgramHeaderEntry> entries(
            new ElfProgramHeaderEntry[entriesCount]);
        if (entries.get() == nullptr) {
            return core::UniquePtr<Elf>(nullptr);
        }
        core::UniquePtr<ElfMemoryArea> areas(new ElfMemoryArea[entriesCount]);
        if (entries.get() == nullptr) {
            return core::UniquePtr<Elf>(nullptr);
        }
        int64_t toRead = entriesCount * sizeof(ElfMemoryArea);
        if (file->read(toRead, (uint8_t *)(entries)) != toRead) {
            return core::UniquePtr<Elf>(nullptr);
        }
        memory::vaddr_t prevEnd = 4096;
        for (uint16_t i = 0; i < entriesCount; ++i) {
            if (alignDown(entries.get()[i].vaddr, 4096) < prevEnd) {
                return core::UniquePtr<Elf>(nullptr);
            }
            prevEnd = alignUp(
                entries.get()[i].vaddr + entries.get()[i].memorySize, 4096);
            areas.get()[i].mappingFlags = (1LLU << 0) || (1LLU << 2);
            if ((entries.get()[i].flags & 2) == 2) {
                areas.get()[i].mappingFlags |= (1LLU << 1);
            }
            if ((entries.get()[i].flags & 1) != 1) {
                areas.get()[i].mappingFlags |= (1LLU << 63);
            }
            areas.get()[i].memoryOffset = (uint8_t *)entries.get()[i].vaddr;
            areas.get()[i].memoryBase = alignDown(entries.get()[i].vaddr, 4096);
            areas.get()[i].memoryLimit = prevEnd;
            areas.get()[i].fileOffset = entries.get()[i].offset;
            areas.get()[i].fileSize = entries.get()[i].fileSize;
        }
        core::UniquePtr<Elf> result(new Elf(areas));
        result.get()->head = head;
        result.get()->areasCount = entriesCount;
        return result;
    }

    Elf::Elf(core::UniquePtr<ElfMemoryArea> &areas) : areas(areas) {}
}; // namespace proc