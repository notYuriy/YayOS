#ifndef __MULTIBOOT_HPP_INCLUDED__
#define __MULTIBOOT_HPP_INCLUDED__

#include <utils.hpp>

namespace multiboot {

    enum BootInfoTagType : uint32_t {
        BasicMemoryInfo = 4,
        BIOSBootDevice = 5,
        BootCommandLine = 1,
        Module = 3,
        ElfSections = 9,
        MemoryMap = 6,
        BootLoaderName = 2,
        APMTable = 10,
        VBEInfo = 11,
        FramebufferInfo = 8,
        EFI32BitSystemTablePointer = 11,
        EFI64BitSystemTablePointer = 12,
        SMBIOSTables = 13,
        ACPIWithOldRSDP = 14,
        ACPIWithNewRSDP = 15,
        NetworkInfo = 16,
        EFIMemoryMap = 17,
        EFIBootServicesNotTerminated = 18,
        EFI32BitImageHandlerPointer = 19,
        EFI64BitImageHandlerPointer = 20,
        ImageLoadBasePhysicalAddress = 21
    };

    enum MemoryMapEntryType : uint32_t {
        EffectivelyUnusable = 0,
        Available = 1,
        UsedByACPI = 3,
        PreserveOnHibernation = 4,
        Defective = 5
    };

#pragma pack(1)
    struct MemoryMapEntry {
        uint64_t baseAddr;
        uint64_t length;
        MemoryMapEntryType type;
        uint32_t : 32;
    };

    struct MemoryMapTag {
        uint32_t type;
        uint32_t size;
        uint32_t entrySize;
        uint32_t entryVersion;
        MemoryMapEntry map[];
        INLINE uint32_t getEntriesCount() const {
            return (size - 16) / sizeof(MemoryMapEntry);
        }
    };

    struct ElfSectionHeader {
        uint32_t name;
        uint32_t type;
        uint64_t flags;
        uint64_t addr;
        uint64_t offset;
        uint64_t size;
        uint32_t link;
        uint32_t info;
        uint64_t addrAlign;
        uint64_t entSize;
    };

    struct ElfSectionsTag {
        uint32_t type;
        uint32_t size;
        uint16_t num;
        uint16_t endSize;
        uint16_t shndx;
        uint16_t reserved;
        uint32_t : 32;
        ElfSectionHeader headers[];
        INLINE uint32_t getEntriesCount() const {
            return (size - 16) / sizeof(ElfSectionHeader);
        }
    };

    struct ModuleTag {
        uint32_t type;
        uint32_t size;
        uint32_t moduleStart;
        uint32_t moduleEnd;
        uint8_t string[];
    };

    struct ACPIWithOldRSDPTag {
        uint32_t type;
        uint32_t size;
        char table[];
    };

    struct ACPIWithNewRSDPTag {
        uint32_t type;
        uint32_t size;
        char table[];
    };

    struct BootInfoTag {
        BootInfoTagType type;
        uint32_t size;
        INLINE BootInfoTag *next() const {
            return (BootInfoTag *)ALIGN_UP(((char *)this) + size, 8);
        }
        template <class T> INLINE T *as() const { return (T *)this; }
        INLINE bool isTerminator() const { return type == 0; }
    };

    struct BootInfoHeader {
        uint32_t totalSize;
        uint32_t reserved;
        BootInfoTag firstTag[];
    };
#pragma pack(1)

}; // namespace multiboot

#endif