#ifndef __GDT_HPP_INCLDUED__
#define __GDT_HPP_INCLUDED__

#include <utils.hpp>

namespace x86_64 {
    constexpr uint64_t GDT_ENTRIES = 7;
#pragma pack(1)
    struct GDTPointer {
        uint16_t size;
        uint64_t *gdt;
    };
#pragma pack(0)
    class GDT {
        static uint64_t m_descriptors[GDT_ENTRIES];
        static GDTPointer m_pointer;

    public:
        static void init();
    };
}; // namespace x86_64

#endif