#ifndef __SDT_HPP_INCLUDED__
#define __SDT_HPP_INCLUDED__

#include <utils.hpp>

namespace acpi {
#pragma packed(1)
    struct SDT {
        char signature[4];
        Uint32 length;
        Uint8 revision;
        Uint8 checksum;
        char OEMID[6];
        char OEMTableID[8];
        Uint32 OEMRevision;
        Uint32 creatorId;
        Uint32 creatorRevision;
        INLINE bool verifyChecksum() {
            Uint8 realChecksum;
            for (Uint64 i = 0; i < length; ++i) {
                realChecksum += ((Uint8*)this)[i];
            }
            return realChecksum == 0;
        }
    };
    static_assert(sizeof(SDT) == 36);
#pragma packed(0)
}; // namespace acpi

#endif