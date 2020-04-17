#ifndef __PIC_HPP_INCLUDED__
#define __PIC_HPP_INCLUDED__

#include <core/interrupts.hpp>
#include <utils.hpp>

namespace drivers {
    class IPIC {
        static IPIC *systemPic;
        static bool picInitialized;

    protected:
        bool picInstanceInitialized;

    public:
        virtual bool registerLegacyIrq(Uint8 irq, core::IDTVector vec) = 0;
        virtual bool enableLegacyIrq(Uint8 irq) = 0;
        virtual bool disableLegacyIrq(Uint8 irq) = 0;
        virtual bool endOfLegacyIrq(Uint8 irq) = 0;

        INLINE bool isInstanceInitialized() { return picInstanceInitialized; }
        INLINE static bool isInitialized() { return picInitialized; }

        INLINE static IPIC *getSystemPIC() { return systemPic; }
        static void detectPIC();
    };

    void setPIC(IPIC *pic);
    IPIC *getPIC();

} // namespace drivers

#endif