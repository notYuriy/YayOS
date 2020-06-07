#ifndef __LOG_HPP_INCLUDED__
#define __LOG_HPP_INCLUDED__

#include <inttypes.hpp>
#include <stdarg.h>

namespace core {
    void putsn(const char *str, uint64_t len);
    void log(const char *fmt, ...);
    void logvarargs(const char *fmt, va_list args);
} // namespace core

#endif