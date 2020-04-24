#ifndef __PORT_IO_HPP_INCLUDED__
#define __PORT_IO_HPP_INCLUDED__

#include <attributes.hpp>
#include <inttypes.hpp>

namespace core {

    class Ports {
    public:
        INLINE static void outb(uint16_t port, uint8_t val) {
            asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
        }
        INLINE static uint8_t inb(uint16_t port) {
            uint8_t ret;
            asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
            return ret;
        }
        INLINE static void waitForIO() {
            asm volatile("outb %%al, $0x80" : : "a"(0));
        }
    };

} // namespace core

#endif