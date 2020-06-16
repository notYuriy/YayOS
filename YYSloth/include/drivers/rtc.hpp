#ifndef __RTC_HPP_INCLUDED__
#define __RTC_HPP_INCLUDED__

#include <core/time.hpp>
#include <utils.hpp>

namespace drivers {
    class RTC {
        enum : uint16_t {
            secondRegister = 0x00,
            minuteRegister = 0x02,
            hourRegister = 0x04,
            dayRegister = 0x07,
            monthRegister = 0x08,
            yearRegister = 0x09,
            updateRegister = 0x0a,
            statusRegister = 0x0b
        };
        static void tryRead(core::TimeInfo *buf);
        static bool updateInProgress();

    public:
        static void read(core::TimeInfo *buf);
    };
}; // namespace drivers

#endif