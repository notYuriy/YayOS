#include <core/interrupts.hpp>
#include <core/portio.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/timer/pit.hpp>

namespace drivers {

    const uint16_t PITIrq = 0;

    void PIT::init(uint32_t frequency) {
        if (!IPIC::isInitialized()) {
            // panic("[PIT] Dependency \"PIC\" is not satisfied\n\r");
        }
        this->m_frequency = frequency;
        uint32_t divisor = 1193180 / frequency;
        if (divisor > 65536) {
            panic("[PIT] Can't handle such a small frequency\n\r");
        }
        core::Ports::outb(0x43, 0x36);
        uint16_t lowDivisor = (uint16_t)divisor;
        core::Ports::outb(0x40, lowDivisor & 0xff);
        core::Ports::outb(0x40, lowDivisor >> 8);
        m_initialized = true;
    }

    bool PIT::enable() { return IPIC::getSystemPIC()->enableLegacyIrq(PITIrq); }

    bool PIT::disable() {
        return IPIC::getSystemPIC()->disableLegacyIrq(PITIrq);
    }

    bool PIT::setCallback(core::IDTVector vec) {
        drivers::IPIC *pic = IPIC::getSystemPIC();
        bool result = pic->registerLegacyIrq(PITIrq, vec);
        return result;
    }

    void PIT::onTerm() { IPIC::getSystemPIC()->endOfLegacyIrq(PITIrq); }

}; // namespace drivers