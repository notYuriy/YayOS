#include <drivers/pic/pic8259.hpp>
#include <x86_64/portio.hpp>

namespace drivers {

    const uint16_t picMasterCommandPort = 0x20;
    const uint16_t picSlaveCommandPort = 0xA0;
    const uint16_t picMasterDataPort = 0x21;
    const uint16_t picSlaveDataPort = 0xA0;

    const uint8_t picEOI = 0x20;
    const uint8_t picMode8086 = 0x01;

    void PIC8259::init() {

        x86_64::Ports::outb(picMasterCommandPort, 0x11);
        x86_64::Ports::waitForIO();
        x86_64::Ports::outb(picSlaveCommandPort, 0x11);
        x86_64::Ports::waitForIO();

        x86_64::Ports::outb(picMasterDataPort, 32);
        x86_64::Ports::waitForIO();
        x86_64::Ports::outb(picSlaveDataPort, 40);
        x86_64::Ports::waitForIO();

        x86_64::Ports::outb(picMasterDataPort, 4);
        x86_64::Ports::waitForIO();
        x86_64::Ports::outb(picSlaveDataPort, 2);
        x86_64::Ports::waitForIO();

        x86_64::Ports::outb(picMasterDataPort, picMode8086);
        x86_64::Ports::waitForIO();
        x86_64::Ports::outb(picMasterDataPort, picMode8086);
        x86_64::Ports::waitForIO();

        m_picMasterMask = 0xff;
        m_picSlaveMask = 0xff;

        x86_64::Ports::outb(picMasterDataPort, m_picMasterMask);
        x86_64::Ports::outb(picSlaveDataPort, m_picSlaveMask);

        asm volatile("sti");

        m_picInstanceInitialized = true;
    }

    bool PIC8259::enableLegacyIrq(uint8_t irq) {
        if (irq < 8) {
            m_picMasterMask &= ~(1 << irq);
        } else {
            m_picSlaveMask &= ~(1 << (irq - 8));
        }
        x86_64::Ports::outb(picMasterDataPort, m_picMasterMask);
        x86_64::Ports::outb(picSlaveDataPort, m_picSlaveMask);
        return true;
    }

    bool PIC8259::disableLegacyIrq(uint8_t irq) {
        if (irq < 8) {
            m_picMasterMask |= (1 << irq);
        } else {
            m_picSlaveMask |= (1 << (irq - 8));
        }
        x86_64::Ports::outb(picMasterDataPort, m_picMasterMask);
        x86_64::Ports::outb(picSlaveDataPort, m_picSlaveMask);
        return true;
    }

    bool PIC8259::endOfLegacyIrq(uint8_t irq) {
        if (irq >= 8) {
            x86_64::Ports::outb(picSlaveCommandPort, picEOI);
        }
        x86_64::Ports::outb(picMasterCommandPort, picEOI);
        return true;
    }

    bool PIC8259::registerLegacyIrq(uint8_t irq, x86_64::IDTVector vec) {
        if (irq >= 16) {
            return false;
        }
        x86_64::IDT::install(irq + 32, vec);
        return true;
    }

} // namespace drivers