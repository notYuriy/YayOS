#include <core/interrupts.hpp>

namespace core {
    IDTEntry IDT::table[256];
    IDTPointer IDT::pointer;
    bool IDT::initialized;

    void intDefaultHandler() { panic("[IDT] Unhandled interrupt\n\r"); }

    extern "C" void intLoadIDT(IDTPointer *pointer);

    void IDT::init() {
        memset(table, 256 * sizeof(IDTEntry), 0);
        pointer.base = (Uint64)(&table);
        pointer.limit = sizeof(table);
        intLoadIDT(&pointer);
        initialized = true;
        for (Uint64 i = 0; i < 256; ++i) {
            install(i, (IDTVector)intDefaultHandler);
        }
    }

    void IDT::install(Uint8 index, IDTVector vec) {
        IDTEntry &entry = table[index];
        entry.addrLow = (Uint16)vec;
        entry.addrMiddle = (Uint16)(vec >> 16);
        entry.addrHigh = (Uint32)(vec >> 32);
        entry.zeroed1 = 0;
        entry.zeroed2 = 0;
        entry.type = 0;
        entry.ist = 0;
        entry.dpl = 0;
        entry.ones1 = 0b111;
        entry.selector = 0x8;
        entry.present = 1;
    }
}; // namespace core