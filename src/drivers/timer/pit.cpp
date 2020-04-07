#include <interrupts.hpp>
#include <pic.hpp>
#include <pit.hpp>
#include <portio.hpp>

namespace drivers {

    Uint16 PITIrq = 0;

    void PIT::init(Uint32 frequency) {
        if (!PIC::isInitialized()) {
            panic("[PICTimer] Dependency \"PIC\" is not satisfied\n\r");
        }
        this->frequency = frequency;
        Uint32 divisor = 1193180 / frequency;
        if (divisor > 65536) {
            panic("[PICTimer] Can't handle such a small frequency\n\r");
        }
        IO::Ports::outb(0x43, 0x36);
        Uint16 lowDivisor = (Uint16)divisor;
        IO::Ports::outb(0x40, lowDivisor & 0xff);
        IO::Ports::outb(0x40, lowDivisor >> 8);
        initialized = true;
    }

    bool PIT::enable() {
        return PIC::getSystemPIC()->enableLegacyIrq(PITIrq);
    }

    bool PIT::disable() {
        return PIC::getSystemPIC()->disableLegacyIrq(PITIrq);
    }

    bool PIT::setCallback(interrupts::IDTVector vec) {
        return PIC::getSystemPIC()->registerLegacyIrq(PITIrq, vec);
    }

}; // namespace drivers