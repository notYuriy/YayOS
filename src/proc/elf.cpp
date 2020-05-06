#include <core/uniqueptr.hpp>
#include <mm/kheap.hpp>
#include <mm/vmmap.hpp>
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
        if (file->read(toRead, (uint8_t *)(entries.get())) != toRead) {
            return core::UniquePtr<Elf>(nullptr);
        }
        memory::vaddr_t prevEnd = 4096;
        for (uint16_t i = 0; i < entriesCount; ++i) {
            if (entries.get()[i].type == ELF_NULL) {
                areas.get()[i].isRequired = false;
                continue;
            }
            if (entries.get()[i].type != ELF_LOAD) {
                return core::UniquePtr<Elf>(nullptr);
            }
            areas.get()[i].isRequired = true;
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
            if (entries.get()[i].offset < 0) {
                core::log("entries.get()[i].offset: %llu\n\r",
                          entries.get()[i].offset);
                core::log("head.headerSize: %llu\n\r", head.headerSize);
                return core::UniquePtr<Elf>(nullptr);
            }
            if (entries.get()[i].fileSize < 0) {
                return core::UniquePtr<Elf>(nullptr);
            }
            areas.get()[i].fileOffset = entries.get()[i].offset;
            areas.get()[i].fileSize = entries.get()[i].fileSize;
        }
        core::UniquePtr<Elf> result(new Elf(areas.move()));
        result.get()->head = head;
        result.get()->areasCount = entriesCount;
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

    bool ElfMemoryArea::unmap() {
        if (isRequired) {
            return memory::VirtualMemoryMapper::freePages(memoryBase,
                                                          memoryLimit);
        }
        return true;
    }

    Elf::Elf(ElfMemoryArea *areas) : areas(areas) {}
    bool Elf::load(fs::IFile *file, memory::UserVirtualAllocator *usralloc) {
        for (uint64_t i = 0; i < areasCount; ++i) {
            if (!usralloc->reserve(areas.get()[i].memoryBase,
                                   areas.get()[i].memoryLimit -
                                       areas.get()[i].memoryBase) ||
                !areas.get()[i].map(file)) {
                for (uint64_t j = 0; j < i; ++j) {
                    areas.get()[i].unmap();
                }
                return false;
            }
        }
        return true;
    }
}; // namespace proc