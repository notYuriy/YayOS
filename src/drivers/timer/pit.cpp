#include <core/interrupts.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/timer/pit.hpp>
#include <core/portio.hpp>

namespace drivers {

    const Uint16 PITIrq = 0;

    void PIT::init(Uint32 frequency) {
        if (!IPIC::isInitialized()) {
            //panic("[PIT] Dependency \"PIC\" is not satisfied\n\r");
        }
        this->frequency = frequency;
        Uint32 divisor = 1193180 / frequency;
        if (divisor > 65536) {
            panic("[PIT] Can't handle such a small frequency\n\r");
        }
        core::Ports::outb(0x43, 0x36);
        Uint16 lowDivisor = (Uint16)divisor;
        core::Ports::outb(0x40, lowDivisor & 0xff);
        core::Ports::outb(0x40, lowDivisor >> 8);
        initialized = true;
    }

    bool PIT::enable() {
        return IPIC::getSystemPIC()->enableLegacyIrq(PITIrq);
    }

    bool PIT::disable() {
        return IPIC::getSystemPIC()->disableLegacyIrq(PITIrq);
    }

    bool PIT::setCallback(core::IDTVector vec) {
        drivers::IPIC* pic = IPIC::getSystemPIC();
        bool result = pic->registerLegacyIrq(PITIrq, vec);
        return result;
    }

    void PIT::onTerm() {
        IPIC::getSystemPIC()->endOfLegacyIrq(PITIrq);
    }

}; // namespace drivers