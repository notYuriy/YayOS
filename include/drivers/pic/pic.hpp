#ifndef __PIC_HPP_INCLUDED__
#define __PIC_HPP_INCLUDED__

#include <interrupts.hpp>
#include <utils.hpp>

namespace drivers {
    class PIC {
        static PIC* systemPic;
        static bool picInitialized;

    protected:
        bool picInstanceInitialized;

    public:
        virtual Uint8 legacyIrq2SystemInt(Uint8 irq) = 0;
        virtual void enableLegacyIrq(Uint8 irq) = 0;
        virtual void disableLegacyIrq(Uint8 irq) = 0;
        virtual void endOfLegacyIrq(Uint8 irq) = 0;

        INLINE bool isInstanceInitialized() { return picInstanceInitialized; }
        INLINE static bool isInitialized() { return picInitialized; }

        INLINE void installLegacyIrqHandler(Uint8 num,
                                            interrupts::IDTVector vec) {
            interrupts::IDT::install(legacyIrq2SystemInt(num), vec);
        };

        INLINE static PIC* getSystemPIC() { return systemPic; }

        static void detectPIC();
    };

    void setPIC(PIC* pic);
    PIC* getPIC();

} // namespace drivers

#endif