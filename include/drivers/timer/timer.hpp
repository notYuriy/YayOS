#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include <core/interrupts.hpp>
#include <utils.hpp>

namespace drivers {
    class ITimer {
    protected:
        bool m_initialized;

    public:
        INLINE bool isInitialized() { return m_initialized; }
        virtual bool setCallback(core::IDTVector callback) = 0;
        virtual bool enable() = 0;
        virtual bool disable() = 0;
        virtual void onTerm() = 0;
        virtual uint64_t getTimeInMilliseconds() = 0;
    };
}; // namespace drivers

#endif