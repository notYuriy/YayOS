#ifndef __MULTIBOOT_HPP_INCLUDED__
#define __MULTIBOOT_HPP_INCLUDED__

#include <utils.hpp>

namespace multiboot {

    enum BootInfoTagType : Uint32 {
        BasicMemoryInfo = 4,
        BIOSBootDevice = 5,
        BootCommandLine = 1,
        Modules = 3,
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

    enum MemoryMapEntryType : Uint32 {
        EffectivelyUnusable = 0,
        Available = 1,
        UsedByACPI = 3,
        PreserveOnHibernation = 4,
        Defective = 5
    };

    struct MemoryMapEntry {
        Uint64 baseAddr;
        Uint64 length;
        MemoryMapEntryType type;
        Uint32 : 32;
    } PACKED;

    struct MemoryMapTag {
        Uint32 type;
        Uint32 size;
        Uint32 entrySize;
        Uint32 entryVersion;
        MemoryMapEntry map[];
        INLINE Uint32 getEntriesCount() const {
            return (size - 16) / sizeof(MemoryMapEntry);
        }
    } PACKED;

    struct ElfSectionHeader {
        Uint32 name;
        Uint32 type;
        Uint64 flags;
        Uint64 addr;
        Uint64 offset;
        Uint64 size;
        Uint32 link;
        Uint32 info;
        Uint64 addrAlign;
        Uint64 entSize;
    } PACKED;

    struct ElfSectionsTag {
        Uint32 type;
        Uint32 size;
        Uint16 num;
        Uint16 endSize;
        Uint16 shndx;
        Uint16 reserved;
        Uint32 : 32;
        ElfSectionHeader headers[];
        INLINE Uint32 getEntriesCount() const {
            return (size - 16) / sizeof(ElfSectionHeader);
        }
    } PACKED;

    struct BootInfoTag {
        BootInfoTagType type;
        Uint32 size;
        INLINE BootInfoTag* next() const {
            return (BootInfoTag*)ALIGN_UP(((char*)this) + size, 8);
        }
        INLINE MemoryMapTag* toMemoryMapTag() const {
            return (MemoryMapTag*)(this);
        }
        INLINE ElfSectionsTag* toElfSectionsTag() const {
            return (ElfSectionsTag*)(this);
        }
        INLINE bool isTerminator() const { return type == 0; }
    } PACKED;

    struct BootInfoHeader {
        Uint32 totalSize;
        Uint32 reserved;
        BootInfoTag firstTag[];
    } PACKED;

}; // namespace multiboot

#endif