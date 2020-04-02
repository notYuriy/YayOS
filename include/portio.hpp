#ifndef __PORT_IO_HPP_INCLUDED__
#define __PORT_IO_HPP_INCLUDED__

#include <attributes.hpp>
#include <inttypes.hpp>

namespace IO {

    class Ports {
    public:
        static INLINE void outb(Uint16 port, Uint8 val) {
            asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
        }
        static INLINE Uint8 inb(Uint16 port) {
            Uint8 ret;
            asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
            return ret;
        }
    };

} // namespace IO

#endif