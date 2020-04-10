#ifndef __LOG_HPP_INCLUDED__
#define __LOG_HPP_INCLUDED__

#include <stdarg.h>

namespace core {
    void log(const char* fmt, ...);
    void logvarargs(const char* fmt, va_list args);
} // namespace core

#endif