#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include <utils.hpp>
#include <interrupts.hpp>

namespace drivers {
    class Timer {
    protected:
        bool initialized;

    public:
        INLINE bool isInitialized() { return initialized; }
        virtual void setCallback(interrupts::IDTVector callback) = 0;
        virtual void enable() =  0;
        virtual void disable() = 0;
    };
}; // namespace drivers

#endif