#include <core/portio.hpp>
#include <drivers/pic/pic8259.hpp>

namespace drivers {

    const uint16_t picMasterCommandPort = 0x20;
    const uint16_t picSlaveCommandPort = 0xA0;
    const uint16_t picMasterDataPort = 0x21;
    const uint16_t picSlaveDataPort = 0xA0;

    const uint8_t picEOI = 0x20;
    const uint8_t picMode8086 = 0x01;

    void PIC8259::init() {

        core::Ports::outb(picMasterCommandPort, 0x11);
        core::Ports::waitForIO();
        core::Ports::outb(picSlaveCommandPort, 0x11);
        core::Ports::waitForIO();

        core::Ports::outb(picMasterDataPort, 32);
        core::Ports::waitForIO();
        core::Ports::outb(picSlaveDataPort, 40);
        core::Ports::waitForIO();

        core::Ports::outb(picMasterDataPort, 4);
        core::Ports::waitForIO();
        core::Ports::outb(picSlaveDataPort, 2);
        core::Ports::waitForIO();

        core::Ports::outb(picMasterDataPort, picMode8086);
        core::Ports::waitForIO();
        core::Ports::outb(picMasterDataPort, picMode8086);
        core::Ports::waitForIO();

        m_picMasterMask = 0xff;
        m_picSlaveMask = 0xff;

        core::Ports::outb(picMasterDataPort, m_picMasterMask);
        core::Ports::outb(picSlaveDataPort, m_picSlaveMask);

        asm volatile("sti");

        m_picInstanceInitialized = true;
    }

    bool PIC8259::enableLegacyIrq(uint8_t irq) {
        if (irq < 8) {
            m_picMasterMask &= ~(1 << irq);
        } else {
            m_picSlaveMask &= ~(1 << (irq - 8));
        }
        core::Ports::outb(picMasterDataPort, m_picMasterMask);
        core::Ports::outb(picSlaveDataPort, m_picSlaveMask);
        return true;
    }

    bool PIC8259::disableLegacyIrq(uint8_t irq) {
        if (irq < 8) {
            m_picMasterMask |= (1 << irq);
        } else {
            m_picSlaveMask |= (1 << (irq - 8));
        }
        core::Ports::outb(picMasterDataPort, m_picMasterMask);
        core::Ports::outb(picSlaveDataPort, m_picSlaveMask);
        return true;
    }

    bool PIC8259::endOfLegacyIrq(uint8_t irq) {
        if (irq >= 8) {
            core::Ports::outb(picSlaveCommandPort, picEOI);
        }
        core::Ports::outb(picMasterCommandPort, picEOI);
        return true;
    }

    bool PIC8259::registerLegacyIrq(uint8_t irq, core::IDTVector vec) {
        if (irq >= 16) {
            return false;
        }
        core::IDT::install(irq + 32, vec);
        return true;
    }

} // namespace drivers