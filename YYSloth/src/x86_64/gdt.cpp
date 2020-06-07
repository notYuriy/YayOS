#include <x86_64/gdt.hpp>
#include <x86_64/tss.hpp>

namespace x86_64 {
    uint64_t GDT::m_descriptors[GDT_ENTRIES];
    GDTPointer GDT::m_pointer;
    extern "C" void loadGDT(GDTPointer *gdt);
    void GDT::init() {
        // null
        m_descriptors[0] = 0;
        // kernel code
        m_descriptors[1] = (1LLU << 44) | (1LLU << 47) | (1LLU << 41LLU) |
                           (1LLU << 43) | (1LLU << 53);
        // kernel data
        m_descriptors[2] = (1LLU << 44) | (1LLU << 47) | (1LLU << 41LLU);
        // user code
        m_descriptors[3] = (1LLU << 44) | (1LLU << 47) | (1LLU << 41LLU) |
                           (1LLU << 43) | (1LLU << 53) | (1LLU << 46) |
                           (1LLU << 45);
        // user data
        m_descriptors[4] = (1LLU << 44) | (1LLU << 47) | (1LLU << 41LLU) |
                           (1LLU << 46) | (1LLU << 45);
        // tss low
        m_descriptors[5] = ((sizeof(TSSLayout) - 1) & 0xffff) |
                           ((TSS::getBase() & 0xffffff) << 16) |
                           (0b1001LLU << 40) | (1LLU << 47) |
                           (((TSS::getBase() >> 24) & 0xff) << 56);
        // tss high
        m_descriptors[6] = TSS::getBase() >> 32;
        m_pointer.gdt = m_descriptors;
        m_pointer.size = sizeof(m_descriptors);
        loadGDT(&m_pointer);
    }
}; // namespace x86_64