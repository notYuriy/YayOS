#ifndef __PIT_HPP_INCLUDED__
#define __PIT_HPP_INCLUDED__

#include <drivers/timer/timer.hpp>

namespace drivers {
    class PIT : public ITimer {
        bool m_initialized;
        uint32_t m_frequency;
        uint64_t m_ticks;

    public:
        void init(uint32_t frequency);
        INLINE bool isInitilaized() { return m_initialized; }
        INLINE uint64_t getFrequency() { return m_frequency; }
        virtual bool setCallback(core::IDTVector callback);
        virtual bool enable();
        virtual bool disable();
        virtual void onTerm();
        virtual uint64_t getTimeInMilliseconds();
    };
} // namespace drivers

#endif