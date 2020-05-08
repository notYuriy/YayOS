#ifndef __PIC_8259_HPP_INCLUDED__
#define __PIC_8259_HPP_INCLUDED__

#include <drivers/pic/pic.hpp>
#include <utils.hpp>

namespace drivers {

    class PIC8259 : public IPIC {
        uint8_t m_picMasterMask;
        uint8_t m_picSlaveMask;

    public:
        void init();
        bool registerLegacyIrq(uint8_t irq, x86_64::IDTVector vec);
        virtual bool enableLegacyIrq(uint8_t irq);
        virtual bool disableLegacyIrq(uint8_t irq);
        virtual bool endOfLegacyIrq(uint8_t irq);
    };

}; // namespace drivers

#endif