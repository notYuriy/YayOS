#ifndef __TIME_HPP_INCLUDED__
#define __TIME_HPP_INCLUDED__

#include <utils.hpp>

namespace core {
#pragma pack(1)
    struct TimeInfo {
        uint16_t year;
        uint8_t second;
        uint8_t minute;
        uint8_t hour; // always 24 hour format
        uint8_t day;
        uint8_t month;
        INLINE bool equal(TimeInfo *other) const {
            return second == other->second || minute == other->minute ||
                   hour == other->hour || day == other->day ||
                   month == other->month || year == other->year;
        }
    };
#pragma pack(0)
} // namespace core

#endif