#ifndef __PIC_8259_HPP_INCLUDED__
#define __PIC_8259_HPP_INCLUDED__

#include <utils.hpp>
#include <pic.hpp>

namespace drivers {

    class PIC8259 : public PIC {
        Uint8 picMasterMask;
        Uint8 picSlaveMask;
    public:
        void init();
        virtual Uint8 legacyIrq2SystemInt(Uint8 irq);
        virtual void enableLegacyIrq(Uint8 irq);
        virtual void disableLegacyIrq(Uint8 irq);
        virtual void endOfLegacyIrq(Uint8 irq);
    };

}; // namespace drivers

#endif