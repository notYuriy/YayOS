#include <interrupts.hpp>


namespace interrupts {
    IdtEntry Idt::table[256];
    IdtPointer Idt::pointer;
    bool Idt::initialized;

    void intDefaultHandler() {
        panic("[Idt] Unhandled interrupt\n\r");
        while (true) {
            asm("pause");
        }
    }
    extern "C" void intLoadIdt(IdtPointer* pointer);

    void Idt::init() {
        memset(table, 256 * sizeof(IdtEntry), 0);
        pointer.base = (Uint64)(&table);
        pointer.limit = sizeof(table);
        intLoadIdt(&pointer);
        initialized = true;
        for(Uint64 i = 0; i < 256; ++i) {
            install(i, (IdtVector)intDefaultHandler);
        }
    }

    void Idt::install(Uint8 index, IdtVector vec) {
        IdtEntry& entry = table[index];
        entry.addrLow = (Uint16)vec;
        entry.addrMiddle = (Uint16)(vec >> 16);
        entry.addrHigh = (Uint32)(vec >> 32);
        entry.zeroed1 = 0;
        entry.zeroed2 = 0;
        entry.zeroed3 = 0;
        entry.type = 0;
        entry.ist = 0;
        entry.dpl = 0;
        entry.ones1 = 0b111;
        entry.selector = 0x8;
        entry.present = 1;
    }

};