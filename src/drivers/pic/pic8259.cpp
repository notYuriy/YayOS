#include <pic8259.hpp>
#include <portio.hpp>

namespace drivers {

    const Uint16 picMasterCommandPort = 0x20;
    const Uint16 picSlaveCommandPort = 0xA0;
    const Uint16 picMasterDataPort = 0x21;
    const Uint16 picSlaveDataPort = 0xA0;

    const Uint8 picEOI = 0x20;
    const Uint8 picMode8086 = 0x01;

    void PIC8259::init() {

        IO::Ports::outb(picMasterCommandPort, 0x11);
        IO::Ports::waitForIO();
        IO::Ports::outb(picSlaveCommandPort, 0x11);
        IO::Ports::waitForIO();

        IO::Ports::outb(picMasterDataPort, 32);
        IO::Ports::waitForIO();
        IO::Ports::outb(picSlaveDataPort, 40);
        IO::Ports::waitForIO();

        IO::Ports::outb(picMasterDataPort, 4);
        IO::Ports::waitForIO();
        IO::Ports::outb(picSlaveDataPort, 2);
        IO::Ports::waitForIO();

        IO::Ports::outb(picMasterDataPort, picMode8086);
        IO::Ports::waitForIO();
        IO::Ports::outb(picMasterDataPort, picMode8086);
        IO::Ports::waitForIO();

        picMasterMask = 0xff;
        picSlaveMask = 0xff;

        IO::Ports::outb(picMasterDataPort, picMasterMask);
        IO::Ports::outb(picSlaveDataPort, picSlaveMask);

        asm volatile("sti");

        picInstanceInitialized = true;
    }

    void PIC8259::enableLegacyIrq(Uint8 irq) {
        if (irq < 8) {
            picMasterMask &= ~(1 << irq);
        } else {
            picSlaveMask &= ~(1 << (irq - 8));
        }
        IO::Ports::outb(picMasterDataPort, picMasterMask);
        IO::Ports::outb(picSlaveDataPort, picSlaveMask);
    }

    void PIC8259::disableLegacyIrq(Uint8 irq) {
        if (irq < 8) {
            picMasterMask |= (1 << irq);
        } else {
            picSlaveMask |= (1 << (irq - 8));
        }
        IO::Ports::outb(picMasterDataPort, picMasterMask);
        IO::Ports::outb(picSlaveDataPort, picSlaveMask);
    }

    void PIC8259::endOfLegacyIrq(Uint8 irq) {
        if(irq >= 8) {
            IO::Ports::outb(picSlaveCommandPort, picEOI);
        }
        IO::Ports::outb(picMasterCommandPort, picEOI);
    }

    Uint8 PIC8259::legacyIrq2SystemInt(Uint8 irq) {
        return irq;
    }

}