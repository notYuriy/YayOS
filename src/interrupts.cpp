#include <interrupts.hpp>

namespace interrupts {
    bool Idt::initialized;
    IdtEntry Idt::table[256];
    IdtPointer Idt::pointer;

    void Idt::init() {
    }

    void Idt::install(Uint8 index, IdtVector handler) {
    }
};