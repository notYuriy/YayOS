#ifndef __PORT_IO_HPP_INCLUDED__
#define __PORT_IO_HPP_INCLUDED__

#include <attributes.hpp>
#include <inttypes.hpp>

namespace IO {

    class Ports {
    public:
        INLINE static void outb(Uint16 port, Uint8 val) {
            asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
        }
        INLINE static Uint8 inb(Uint16 port) {
            Uint8 ret;
            asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
            return ret;
        }
        INLINE static void waitForIO() {
            asm volatile("outb %%al, $0x80" : : "a"(0));
        }
    };

} // namespace IO

#endif