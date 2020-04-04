#include <interrupts.hpp>
#include <pic.hpp>
#include <pictimer.hpp>
#include <portio.hpp>

namespace drivers {

    Uint16 PICTimerIrq = 0;

    void PICTimer::init(Uint32 frequency) {
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

    void PICTimer::enable() {
        PIC::getSystemPIC()->enableLegacyIrq(PICTimerIrq);
    }

    void PICTimer::disable() {
        PIC::getSystemPIC()->disableLegacyIrq(PICTimerIrq);
    }

    void PICTimer::setCallback(interrupts::IDTVector vec) {
        PIC::getSystemPIC()->installLegacyIrqHandler(PICTimerIrq, vec);
    }

}; // namespace drivers