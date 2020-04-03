#ifndef __INTERRUPTS_HPP_INCLUDED__
#define __INTERRUPTS_HPP_INCLUDED__

#include <utils.hpp>

namespace interrupts {

    #pragma pack(1)
    struct IdtEntry {
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
    #pragma pack()

    static_assert(sizeof(IdtEntry) == 16);

    #pragma pack(1)
    struct IdtPointer {
        Uint16 limit;
        Uint64 base;
    };
    #pragma pack(0)

    static_assert(sizeof(IdtPointer) == 10);

    typedef Uint64 IdtVector;

    class Idt {
        static IdtPointer pointer;
        static IdtEntry table[256];
        static bool initialized;

    public:
        INLINE static bool isInitialized() { return initialized; }
        void init();
        void install(Uint8 index, IdtVector handler);
    };

}; // namespace interrupts

#endif