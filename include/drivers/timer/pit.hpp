#ifndef __PIT_HPP_INCLUDED__
#define __PIT_HPP_INCLUDED__

#include <timer.hpp>

namespace drivers {
    class PIT : public Timer {
        bool initialized;
        Uint32 frequency;
    public:
        void init(Uint32 frequency);
        INLINE bool isInitilaized() { return initialized; }
        INLINE Uint64 getFrequency() { return frequency; }
        virtual bool setCallback(core::IDTVector callback);
        virtual bool enable();
        virtual bool disable();
        virtual void onTerm();
    };
} // namespace drivers

#endif