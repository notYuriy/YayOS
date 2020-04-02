#ifndef __KPRINTF_HPP_INCLUDED__
#define __KPRINTF_HPP_INCLUDED__

#include <stdarg.h>

void kprintf(const char* fmt, ...);
void vkprintf(const char* fmt, va_list args);

#endif