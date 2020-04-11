#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include <utils.hpp>
#include <core/interrupts.hpp>

namespace drivers {
    class ITimer {
    protected:
        bool initialized;

    public:
        INLINE bool isInitialized() { return initialized; }
        virtual bool setCallback(core::IDTVector callback) = 0;
        virtual bool enable() =  0;
        virtual bool disable() = 0;
        virtual void onTerm() = 0;
    };
}; // namespace drivers

#endif