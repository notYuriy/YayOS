#include <drivers/serial/serial.hpp>
#include <x86_64/portio.hpp>

namespace drivers {

    bool Serial::readyToSend(SerialPort port) {
        return (x86_64::Ports::inb(port + 5) & 0x20);
    }

    bool Serial::readyToRecieve(SerialPort port) {
        return (x86_64::Ports::inb(port + 5) & 0x01) == 1;
    }

    void Serial::init(SerialPort port) {
        x86_64::Ports::outb(port + 1, 0x00);
        x86_64::Ports::outb(port + 3, 0x80);
        x86_64::Ports::outb(port + 0, 0x03);
        x86_64::Ports::outb(port + 1, 0x00);
        x86_64::Ports::outb(port + 3, 0x03);
        x86_64::Ports::outb(port + 2, 0xC7);
        x86_64::Ports::outb(port + 4, 0x0B);
    }

    void Serial::send(SerialPort port, uint8_t byte) {
        while (!Serial::readyToSend(port)) {
            asm("pause");
        }
        x86_64::Ports::outb(port, byte);
    }

    uint8_t Serial::recieve(SerialPort port, bool nb) {
        if (!nb) {
            while (!Serial::readyToRecieve(port)) {
                asm("pause");
            }
        }
        return x86_64::Ports::inb(port);
    }

} // namespace drivers