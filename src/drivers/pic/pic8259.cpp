#include <core/portio.hpp>
#include <drivers/pic/pic8259.hpp>

namespace drivers {

    const Uint16 picMasterCommandPort = 0x20;
    const Uint16 picSlaveCommandPort = 0xA0;
    const Uint16 picMasterDataPort = 0x21;
    const Uint16 picSlaveDataPort = 0xA0;

    const Uint8 picEOI = 0x20;
    const Uint8 picMode8086 = 0x01;

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

        picMasterMask = 0xff;
        picSlaveMask = 0xff;

        core::Ports::outb(picMasterDataPort, picMasterMask);
        core::Ports::outb(picSlaveDataPort, picSlaveMask);

        asm volatile("sti");

        picInstanceInitialized = true;
    }

    bool PIC8259::enableLegacyIrq(Uint8 irq) {
        if (irq < 8) {
            picMasterMask &= ~(1 << irq);
        } else {
            picSlaveMask &= ~(1 << (irq - 8));
        }
        core::Ports::outb(picMasterDataPort, picMasterMask);
        core::Ports::outb(picSlaveDataPort, picSlaveMask);
        return true;
    }

    bool PIC8259::disableLegacyIrq(Uint8 irq) {
        if (irq < 8) {
            picMasterMask |= (1 << irq);
        } else {
            picSlaveMask |= (1 << (irq - 8));
        }
        core::Ports::outb(picMasterDataPort, picMasterMask);
        core::Ports::outb(picSlaveDataPort, picSlaveMask);
        return true;
    }

    bool PIC8259::endOfLegacyIrq(Uint8 irq) {
        if (irq >= 8) {
            core::Ports::outb(picSlaveCommandPort, picEOI);
        }
        core::Ports::outb(picMasterCommandPort, picEOI);
        return true;
    }

    bool PIC8259::registerLegacyIrq(Uint8 irq, core::IDTVector vec) {
        if (irq >= 16) {
            return false;
        }
        core::IDT::install(irq + 32, vec);
        return true;
    }

} // namespace drivers