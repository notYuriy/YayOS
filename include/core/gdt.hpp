#ifndef __GDT_HPP_INCLDUED__
#define __GDT_HPP_INCLUDED__

#include <utils.hpp>

namespace core {
    constexpr uint64_t GDT_MAX_ENTRIES = 7;
#pragma pack(1)
    struct GDTPointer {
        uint16_t size;
        uint64_t *gdt;
    };
#pragma pack(0)
    class GDT {
        static uint64_t m_descriptors[GDT_MAX_ENTRIES];
        static GDTPointer m_pointer;

    public:
        static void init();
    };
}; // namespace core

#endif