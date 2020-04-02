#include <portio.hpp>
#include <serial.hpp>

namespace IO {

    bool Serial::readyToSend(SerialPort port) {
        return (Ports::inb(port + 5) & 0x20);
    }

    bool Serial::readyToRecieve(SerialPort port) {
        return (Ports::inb(port + 5) & 0x01) == 1;
    }

    void Serial::init(SerialPort port) {
        Ports::outb(port + 1, 0x00);
        Ports::outb(port + 3, 0x80);
        Ports::outb(port + 0, 0x03);
        Ports::outb(port + 1, 0x00);
        Ports::outb(port + 3, 0x03);
        Ports::outb(port + 2, 0xC7);
        Ports::outb(port + 4, 0x0B);
    }

    void Serial::send(SerialPort port, Uint8 byte) {
        while (!Serial::readyToSend(port)) {
            asm("pause");
        }
        Ports::outb(port, byte);
    }

    Uint8 Serial::recieve(SerialPort port) {
        while (!Serial::readyToRecieve(port)) {
            asm("pause");
        }
        return Ports::inb(port);
    }

} // namespace IO