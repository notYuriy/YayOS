#ifndef __RSDP_HPP_INCLUDED__
#define __RSDP_HPP_INCLUDED__

#include <utils.hpp>
#include <multiboot.hpp>
#include <physbase.hpp>

namespace acpi {

#pragma pack(1)
    struct RSDPv1 {
        char signature[8];
        Uint8 checksum;
        char OEMID[6];
        Uint8 revision;
        memory::PAddr32 rsdtAddress;
    };
    static_assert(sizeof(RSDPv1) == 20);
#pragma pack(0)

#pragma pack(1)
    struct RSDPv2 : RSDPv1 {
        Uint32 length;
        memory::PAddr xsdtAddress;
        Uint8 extendedChecksum;
        Uint8 reserved[3];
    };
    static_assert(sizeof(RSDPv2) == 36);
#pragma pack(0)

    class RSDPManager {
        static RSDPv1* v1;
        static RSDPv2* v2;
        static Uint8 version;
        static bool initialized;
    public:
        INLINE static bool isInitialized() { return initialized; }
        static void init(multiboot::BootInfoHeader* info);
        INLINE static Uint8 getVersion() { return version; }
        INLINE static RSDPv1* getV1() {
            if (version == 2) {
                return (RSDPv1*)v2;
            }
            return v1;
        }
        INLINE static RSDPv2* getV2() {
            return v2;
        }
    };

};

#endif