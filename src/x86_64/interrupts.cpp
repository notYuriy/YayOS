#include <x86_64/interrupts.hpp>

namespace x86_64 {
    IDTEntry IDT::m_table[256];
    IDTPointer IDT::m_pointer;
    bool IDT::m_initialized;

    void intDefaultHandler() { panic("[IDT] Unhandled interrupt\n\r"); }

    extern "C" void intLoadIDT(IDTPointer *pointer);

    void IDT::init() {
        memset(m_table, 256 * sizeof(IDTEntry), 0);
        m_pointer.base = (uint64_t)(&m_table);
        m_pointer.limit = sizeof(m_table);
        intLoadIDT(&m_pointer);
        m_initialized = true;
        for (uint64_t i = 0; i < 256; ++i) {
            install(i, (IDTVector)intDefaultHandler);
        }
    }

    void IDT::install(uint8_t index, IDTVector vec) {
        IDTEntry &entry = m_table[index];
        entry.addrLow = (uint16_t)vec;
        entry.addrMiddle = (uint16_t)(vec >> 16);
        entry.addrHigh = (uint32_t)(vec >> 32);
        entry.zeroed1 = 0;
        entry.zeroed2 = 0;
        entry.type = 0;
        entry.ist = 0;
        entry.dpl = 0;
        entry.ones1 = 0b111;
        entry.selector = 0x8;
        entry.present = 1;
    }
}; // namespace x86_64