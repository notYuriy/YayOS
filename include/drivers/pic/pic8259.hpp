#ifndef __PIC_8259_HPP_INCLUDED__
#define __PIC_8259_HPP_INCLUDED__

#include <drivers/pic/pic.hpp>
#include <utils.hpp>

namespace drivers {

    class PIC8259 : public IPIC {
        Uint8 picMasterMask;
        Uint8 picSlaveMask;

    public:
        void init();
        bool registerLegacyIrq(Uint8 irq, core::IDTVector vec);
        virtual bool enableLegacyIrq(Uint8 irq);
        virtual bool disableLegacyIrq(Uint8 irq);
        virtual bool endOfLegacyIrq(Uint8 irq);
    };

}; // namespace drivers

#endif