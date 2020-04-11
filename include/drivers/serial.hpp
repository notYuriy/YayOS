#ifndef __SERIAL_HPP_INCLUDED__
#define __SERIAL_HPP_INCLUDED__

#include <inttypes.hpp>
#include <core/portio.hpp>
#include <drivers/serial.hpp>
#include <stdarg.h>

namespace drivers {

    enum SerialPort : Uint16 {
        COM1 = 0x3f8,
        COM2 = 0x2f8,
        COM3 = 0x3e8,
        COM4 = 0x2e8
    };

    class Serial {
        static bool readyToSend(SerialPort port);
        static bool readyToRecieve(SerialPort port);

    public:
        static void init(SerialPort port);
        static void send(SerialPort port, Uint8 byte);
        static Uint8 recieve(SerialPort port);
    };

} // namespace IO

#endif