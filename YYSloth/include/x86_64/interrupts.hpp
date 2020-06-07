#ifndef __INTERRUPTS_HPP_INCLUDED__
#define __INTERRUPTS_HPP_INCLUDED__

#include <utils.hpp>

namespace x86_64 {

#pragma pack(1)
    struct IDTEntry {
        uint16_t addrLow;
        uint16_t selector;
        uint8_t ist : 3;
        uint8_t zeroed1 : 5;
        bool intsEnabled : 1;
        uint8_t ones1 : 3;
        uint8_t zeroed2 : 1;
        uint8_t dpl : 2;
        uint8_t present : 1;
        uint16_t addrMiddle;
        uint32_t addrHigh;
        uint32_t zeroed3;
    };

    static_assert(sizeof(IDTEntry) == 16);

    struct IDTPointer {
        uint16_t limit;
        uint64_t base;
    };
#pragma pack(0)

    static_assert(sizeof(IDTPointer) == 10);

    typedef uint64_t IDTVector;

    class IDT {
        static IDTPointer m_pointer;
        static IDTEntry m_table[256];
        static bool m_initialized;

    public:
        INLINE static bool isInitialized() { return m_initialized; }
        static void init();
        static void install(uint8_t index, IDTVector handler, uint8_t cpl = 0,
                            bool disableInts = true);
    };

}; // namespace x86_64

#endif