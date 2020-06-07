#ifndef __TIMER_HPP_INCLUDED__
#define __TIMER_HPP_INCLUDED__

#include <utils.hpp>
#include <x86_64/interrupts.hpp>

namespace drivers {
    class ITimer {
    protected:
        bool m_initialized;

    public:
        INLINE bool isInitialized() { return m_initialized; }
        virtual bool setCallback(x86_64::IDTVector callback) = 0;
        virtual bool enable() = 0;
        virtual bool disable() = 0;
        virtual void onTerm() = 0;
        virtual uint64_t getTimeInMilliseconds() = 0;
    };
}; // namespace drivers

#endif