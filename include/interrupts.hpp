#ifndef __INTERRUPTS_HPP_INCLUDED__
#define __INTERRUPTS_HPP_INCLUDED__

#include <utils.hpp>

namespace interrupts {

#pragma pack(1)
    struct IDTEntry {
        Uint16 addrLow;
        Uint16 selector;
        Uint8 ist : 3;
        Uint8 zeroed1 : 5;
        Uint8 type : 1;
        Uint8 ones1 : 3;
        Uint8 zeroed2 : 1;
        Uint8 dpl : 2;
        Uint8 present : 1;
        Uint16 addrMiddle;
        Uint32 addrHigh;
        Uint32 zeroed3;
    };
#pragma pack(0)

    static_assert(sizeof(IDTEntry) == 16);

#pragma pack(1)
    struct IDTPointer {
        Uint16 limit;
        Uint64 base;
    };
#pragma pack(0)

    static_assert(sizeof(IDTPointer) == 10);

    typedef Uint64 IDTVector;

    class IDT {
        static IDTPointer pointer;
        static IDTEntry table[256];
        static bool initialized;

    public:
        INLINE static bool isInitialized() { return initialized; }
        static void init();
        static void install(Uint8 index, IDTVector handler);
    };

}; // namespace interrupts

#endif