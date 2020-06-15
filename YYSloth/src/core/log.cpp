#include <drivers/serial/serial.hpp>
#include <inttypes.hpp>
#include <stdarg.h>

using namespace drivers;

namespace core {

    char getCharFromDigit(uint8_t digit) {
        if (digit >= 10) {
            return 'A' - 10 + digit;
        }
        return '0' + digit;
    }

    void printUint64Rec(uint64_t num, uint8_t base) {
        if (num == 0) {
            return;
        }
        printUint64Rec(num / base, base);
        Serial::send(SerialPort::COM1, getCharFromDigit(num % base));
    }

    void printUint64(uint64_t val, uint8_t base) {
        if (val == 0) {
            Serial::send(SerialPort::COM1, '0');
        } else {
            printUint64Rec(val, base);
        }
    }

    void printInt64(int64_t val, uint8_t base) {
        uint64_t abs;
        if (val < 0) {
            Serial::send(SerialPort::COM1, '-');
            abs = 0 - val;
            printUint64(abs, base);
        } else {
            abs = val;
            printUint64(abs, base);
        }
    }

    void printPointer(uint64_t pointer, int depth) {
        if (depth == 0) {
            return;
        }
        printPointer(pointer / 16, depth - 1);
        Serial::send(SerialPort::COM1, getCharFromDigit(pointer % 16));
    }

    void puts(char *str) {
        for (uint64_t i = 0; str[i] != '\0'; ++i) {
            Serial::send(SerialPort::COM1, str[i]);
        }
    }

    void putsn(const char *str, uint64_t len) {
        for (uint64_t i = 0; i < len; ++i) {
            Serial::send(SerialPort::COM1, str[i]);
        }
    }

    void logVarargs(const char *fmt, va_list args) {
        for (uint64_t i = 0; fmt[i] != '\0'; ++i) {
            if (fmt[i] != '%') {
                Serial::send(SerialPort::COM1, fmt[i]);
            } else {
                ++i;
                switch (fmt[i]) {
                case '%':
                    Serial::send(SerialPort::COM1, '%');
                    break;
                case 'd':
                    printInt64(va_arg(args, int32_t), 10);
                    break;
                case 'u':
                    printUint64(va_arg(args, uint32_t), 10);
                    break;
                case 'p':
                    printPointer(va_arg(args, uint64_t), 16);
                    break;
                case 's':
                    puts(va_arg(args, char *));
                    break;
                case 'c':
                    Serial::send(SerialPort::COM1, (char)va_arg(args, int));
                    break;
                case 'l': {
                    ++i;
                    switch (fmt[i]) {
                    case 'u':
                        printUint64(va_arg(args, uint64_t), 10);
                        break;
                    case 'd':
                        printInt64(va_arg(args, int64_t), 10);
                        break;
                    case 'l': {
                        ++i;
                        switch (fmt[i]) {
                        case 'u':
                            printUint64(va_arg(args, uint64_t), 10);
                            break;
                        case 'd':
                            printInt64(va_arg(args, int64_t), 10);
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

    void log(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        logVarargs(fmt, args);
        va_end(args);
    }

}; // namespace core