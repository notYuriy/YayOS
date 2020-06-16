#ifndef __CMOS_HPP_INCLUDED__
#define __CMOS_HPP_INCLUDED__

#include <utils.hpp>
#include <x86_64/portio.hpp>

namespace drivers {
    class CMOS {
    public:
        static const uint64_t addressPort = 0x70;
        static const uint64_t dataPort = 0x71;
        INLINE static uint8_t read(uint8_t reg) {
            x86_64::Ports::outb(addressPort, reg);
            return x86_64::Ports::inb(dataPort);
        }
    };
}; // namespace drivers

#endif