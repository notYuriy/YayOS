#ifndef __ELF_HPP_INCLUDED__
#define __ELF_HPP_INCLUDED__

#include <core/dynarray.hpp>
#include <fs/vfs.hpp>
#include <memory/physbase.hpp>
#include <memory/usrvmmngr.hpp>
#include <memory/vmbase.hpp>
#include <utils.hpp>

namespace proc {

    // http://ftp.openwatcom.org/devel/docs/elf-64-gen.pdf
    constexpr uint8_t ELF_SYSV_ABI = 0;
    constexpr uint8_t ELF_SYSV_ABI_VERSION = 0;
    constexpr uint8_t ELF_CLASS_64 = 2;
    constexpr uint8_t ELF_DATA_LSB = 1;
    constexpr uint8_t ELF_EXEC = 2;
    constexpr uint8_t ELF_CUR_VERSION = 1;
    constexpr uint8_t ELF_ARCH_X86_64 = 0x3e;
    constexpr uint16_t ELF_HEADER_SIZE = 64;
    constexpr uint32_t ELF_NULL = 0;
    constexpr uint32_t ELF_LOAD = 1;

#pragma pack(1)
    struct ElfHeader {
        char magic[4];
        uint8_t fileClass;
        uint8_t dataEncoding;
        uint8_t fileVersion;
        uint8_t ABI;
        uint8_t ABIVersion;
        uint8_t padding[6];
        uint8_t identSize;
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        memory::vaddr_t entryPoint;
        int64_t programHeaderOffset;
        int64_t sectionHeaderOffset;
        uint32_t flags;
        uint16_t headerSize;
        uint16_t programEntrySize;
        uint16_t programEntryCount;
        uint16_t sectionEntrySize;
        uint16_t sectionEntryCount;
        uint16_t stringSectionIndex;

        bool verify();
    };

    // linux man page says that they are sorted
    // by virtual address, so no need to sort them
    // on overlap check
    struct ElfProgramHeaderEntry {
        uint32_t type;
        uint32_t flags;
        int64_t offset;
        memory::vaddr_t vaddr;
        memory::paddr_t paddr;
        int64_t fileSize;
        uint64_t memorySize;
        uint64_t align;
    };
#pragma pack(0)

    static_assert(sizeof(ElfHeader) == 64);
    static_assert(sizeof(ElfProgramHeaderEntry) == 56);

    struct ElfMemoryArea {
        uint64_t mappingFlags;
        memory::vaddr_t memoryBase;
        uint8_t *memoryOffset;
        memory::vaddr_t memoryLimit;
        uint64_t fileOffset;
        int64_t fileSize;
        bool isRequired;
        bool map(fs::IFile *file);
        void unmap();
    };

    struct Elf {
        ElfHeader head;
        uint16_t areasCount;
        ElfMemoryArea *areas;
        bool load(fs::IFile *file, memory::UserVirtualAllocator *allocator);
    };

    Elf *parseElf(fs::IFile *file);

}; // namespace proc

#endif