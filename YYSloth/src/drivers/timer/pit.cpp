#include <drivers/pic/pic.hpp>
#include <drivers/timer/pit.hpp>
#include <x86_64/interrupts.hpp>
#include <x86_64/portio.hpp>

namespace drivers {

    const uint16_t PITIrq = 0;

    void PIT::init(uint32_t frequency) {
        if (!IPIC::isInitialized()) {
            panic("[PIT] Dependency \"PIC\" is not satisfied\n\r");
        }
        this->m_frequency = frequency;
        uint32_t divisor = 1193180 / frequency;
        if (divisor > 65536) {
            panic("[PIT] Can't handle such a small frequency\n\r");
        }
        x86_64::Ports::outb(0x43, 0x36);
        uint16_t lowDivisor = (uint16_t)divisor;
        x86_64::Ports::outb(0x40, lowDivisor & 0xff);
        x86_64::Ports::outb(0x40, lowDivisor >> 8);
        m_ticks = 0;
        m_initialized = true;
    }

    bool PIT::enable() { return IPIC::getSystemPIC()->enableLegacyIrq(PITIrq); }

    bool PIT::disable() {
        return IPIC::getSystemPIC()->disableLegacyIrq(PITIrq);
    }

    bool PIT::setCallback(x86_64::IDTVector vec) {
        drivers::IPIC *pic = IPIC::getSystemPIC();
        bool result = pic->registerLegacyIrq(PITIrq, vec);
        return result;
    }

    void PIT::onTerm() {
        IPIC::getSystemPIC()->endOfLegacyIrq(PITIrq);
        m_ticks++;
    }

    uint64_t PIT::getTimeInMilliseconds() {
        return m_ticks * 1000 / m_frequency;
    }

}; // namespace drivers