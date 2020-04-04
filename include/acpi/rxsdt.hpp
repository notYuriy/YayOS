#ifndef __RXSDT_HPP_INCLDUED__
#define __RXSDT_HPP_INCLUDED__

#include <physbase.hpp>
#include <sdt.hpp>

namespace acpi {
    struct RSDTRaw : SDT {
        memory::PAddr32 pAddrs[];
    };
    struct XSDTRaw : SDT {
        memory::PAddr pAddrs[];
    };
    struct RootTable : SDT {
        SDT* tables[];
    };
    class RootTableManager {
        static bool initialized;
        static RootTable* table;

    public:
        INLINE static bool isInitialized() { return initialized; }
        static void init();
    };
}; // namespace acpi

#endif