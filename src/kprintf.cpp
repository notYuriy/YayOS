#include <inttypes.hpp>
#include <serial.hpp>
#include <stdarg.h>

using namespace IO;

char getCharFromDigit(Uint8 digit) {
    if (digit >= 10) {
        return 'A' - 10 + digit;
    }
    return '0' + digit;
}

void printUint64Rec(Uint64 num, Uint8 base) {
    if (num == 0) {
        return;
    }
    printUint64Rec(num / base, base);
    Serial::send(SerialPort::COM1, getCharFromDigit(num % base));
}

void printUint64(Uint64 val, Uint8 base) {
    if (val == 0) {
        Serial::send(SerialPort::COM1, '0');
    } else {
        printUint64Rec(val, base);
    }
}

void printInt64(Int64 val, Uint8 base) {
    Uint64 abs;
    if (val < 0) {
        Serial::send(SerialPort::COM1, '-');
        abs = 0 - val;
        printUint64(abs, base);
    } else {
        abs = val;
        printUint64(abs, base);
    }
}

void printPointer(Uint64 pointer, int depth) {
    if (depth == 0) {
        return;
    }
    printPointer(pointer / 16, depth - 1);
    Serial::send(SerialPort::COM1, getCharFromDigit(pointer % 16));
}

void puts(char* str) {
    for (Uint64 i = 0; str[i] != '\0'; ++i) {
        Serial::send(SerialPort::COM1, str[i]);
    }
}

void putsn(char* str, Uint64 len) {
    for (Uint64 i = 0; i < len; ++i) {
        Serial::send(SerialPort::COM1, str[i]);
    }
}

void vkprintf(const char* fmt, va_list args) {
    for (Uint64 i = 0; fmt[i] != '\0'; ++i) {
        if (fmt[i] != '%') {
            Serial::send(SerialPort::COM1, fmt[i]);
        } else {
            ++i;
            switch (fmt[i]) {
            case '%':
                Serial::send(SerialPort::COM1, '%');
                break;
            case 'd':
                printInt64(va_arg(args, Int32), 10);
                break;
            case 'u':
                printUint64(va_arg(args, Uint32), 10);
                break;
            case 'p':
                printPointer(va_arg(args, Uint64), 16);
                break;
            case 's':
                puts(va_arg(args, char*));
                break;
            case 'c':
                Serial::send(SerialPort::COM1, (char)va_arg(args, int));
                break;
            case 'l': {
                ++i;
                switch (fmt[i]) {
                case 'u':
                    printUint64(va_arg(args, Uint64), 10);
                    break;
                case 'd':
                    printInt64(va_arg(args, Int64), 10);
                    break;
                case 'l': {
                    ++i;
                    switch (fmt[i]) {
                    case 'u':
                        printUint64(va_arg(args, Uint64), 10);
                        break;
                    case 'd':
                        printInt64(va_arg(args, Int64), 10);
                        break;
                    default:
                        break;
                    }
                }
                default:
                    break;
                }
            }
            default:
                break;
            }
        }
    }
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprintf(fmt, args);
    va_end(args);
}