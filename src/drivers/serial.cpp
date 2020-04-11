#include <core/portio.hpp>
#include <drivers/serial.hpp>

namespace drivers {

    bool Serial::readyToSend(SerialPort port) {
        return (core::Ports::inb(port + 5) & 0x20);
    }

    bool Serial::readyToRecieve(SerialPort port) {
        return (core::Ports::inb(port + 5) & 0x01) == 1;
    }

    void Serial::init(SerialPort port) {
        core::Ports::outb(port + 1, 0x00);
        core::Ports::outb(port + 3, 0x80);
        core::Ports::outb(port + 0, 0x03);
        core::Ports::outb(port + 1, 0x00);
        core::Ports::outb(port + 3, 0x03);
        core::Ports::outb(port + 2, 0xC7);
        core::Ports::outb(port + 4, 0x0B);
    }

    void Serial::send(SerialPort port, Uint8 byte) {
        while (!Serial::readyToSend(port)) {
            asm("pause");
        }
        core::Ports::outb(port, byte);
    }

    Uint8 Serial::recieve(SerialPort port) {
        while (!Serial::readyToRecieve(port)) {
            asm("pause");
        }
        return core::Ports::inb(port);
    }

} // namespace IO