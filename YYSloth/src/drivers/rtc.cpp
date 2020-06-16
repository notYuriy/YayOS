#include <drivers/cmos.hpp>
#include <drivers/rtc.hpp>

namespace drivers {

    void RTC::tryRead(core::TimeInfo *info) {
        info->second = CMOS::read(secondRegister);
        info->minute = CMOS::read(minuteRegister);
        info->hour = CMOS::read(hourRegister);
        info->day = CMOS::read(dayRegister);
        info->month = CMOS::read(monthRegister);
        info->year = CMOS::read(yearRegister);
    }

    bool RTC::updateInProgress() { return (CMOS::read(0x0a) & 0x80) != 0; }

    void RTC::read(core::TimeInfo *info) {
        core::TimeInfo buf;
        do {
            while (updateInProgress()) {
                asm("pause");
            }
            tryRead(&buf);
            tryRead(info);
        } while (!info->equal(&buf));
        uint8_t status = CMOS::read(0x0b);
        if (!(status & 0x04)) {
            info->second = (info->second & 0x0f) + ((info->second / 16) * 10);
            info->minute = (info->minute & 0x0f) + ((info->minute / 16) * 10);
            info->day = (info->day & 0x0f) + ((info->day / 16) * 10);
            info->month = (info->month & 0x0f) + ((info->month / 16) * 10);
            info->year = (info->year & 0x0f) + ((info->year / 16) * 10);
            info->hour =
                ((info->hour & 0x0f) + (((info->hour & 0x70) / 16) * 10)) |
                (info->hour & 0x80);
        }
        if (!(status & 0x02) && (info->hour & 0x80)) {
            info->hour = ((info->hour & 0x7f) + 12) % 24;
        }
        info->year += 2000;
        if (info->year < 2020) {
            panic("[RTC] Reading year returned anomalous value (< 2020)\n\r");
        }
    }

}; // namespace drivers