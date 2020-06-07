#ifndef __PIC_HPP_INCLUDED__
#define __PIC_HPP_INCLUDED__

#include <utils.hpp>
#include <x86_64/interrupts.hpp>

namespace drivers {
    class IPIC {
        static IPIC *m_systemPic;
        static bool m_picInitialized;

    protected:
        bool m_picInstanceInitialized;

    public:
        virtual bool registerLegacyIrq(uint8_t irq, x86_64::IDTVector vec) = 0;
        virtual bool enableLegacyIrq(uint8_t irq) = 0;
        virtual bool disableLegacyIrq(uint8_t irq) = 0;
        virtual bool endOfLegacyIrq(uint8_t irq) = 0;

        INLINE bool isInstanceInitialized() { return m_picInstanceInitialized; }
        INLINE static bool isInitialized() { return m_picInitialized; }

        INLINE static IPIC *getSystemPIC() { return m_systemPic; }
        static void detectPIC();
    };

    void setPIC(IPIC *pic);
    IPIC *getPIC();

} // namespace drivers

#endif