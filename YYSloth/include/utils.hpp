#ifndef __UTILS_HPP_INCLUDED__
#define __UTILS_HPP_INCLUDED__

#include <attributes.hpp>
#include <core/cpprt.hpp>
#include <core/log.hpp>
#include <cpuid.h>
#include <inttypes.hpp>
#include <stdarg.h>

#define KB *1024
#define MB *1024 * 1024
#define GB *1024 * 1024 * 1024

INLINE static uint64_t alignUp(uint64_t value, uint64_t align) {
    return ((value + align - 1) / align) * align;
}

INLINE static uint64_t alignDown(uint64_t value, uint64_t align) {
    return (value / align) * align;
}

[[noreturn]] INLINE void panic(const char *msg) {
    asm __volatile__("cli");
    core::log("\u001b[31m");
    core::log(msg);
    while (1) {
        asm("pause");
    }
}

// Used for fast bitmap scanning. Source is taken from
// https://www.chessprogramming.org/BitScan#Bsf.2FBsr_x86-64_Timings
INLINE uint8_t bitScanForward(uint64_t x) {
    asm __volatile__("bsfq %0, %0" : "=r"(x) : "0"(x));
    return (uint8_t)x;
}

INLINE void atomicIncrement(uint64_t *p) {
    asm __volatile__("lock incq %0" : "=m"(p) : "m"(p));
}

INLINE void atomicDecrement(uint64_t *p) {
    asm __volatile__("lock decq %0" : "=m"(p) : "m"(p));
}

#define ALIGN_UP(x, align) (alignUp((uint64_t)x, (uint64_t)align))
#define ALIGN_DOWN(x, align) (alignUp((uint64_t)x, (uint64_t)align))

INLINE static void zeroPage(void *addr) {
    uint64_t *p = (uint64_t *)addr;
    for (uint16_t i = 0; i < 512; ++i) {
        p[i] = 0;
    }
}

INLINE static void memcpy(void *dst, const void *src, uint64_t size) {
    for (uint64_t i = 0; i < size; ++i) {
        ((char *)dst)[i] = ((char *)src)[i];
    }
}

INLINE static void memset(void *dst, uint64_t size, uint8_t fill) {
    for (uint64_t i = 0; i < size; ++i) {
        ((char *)dst)[i] = fill;
    }
}

INLINE static bool streq(const char *str1, const char *str2) {
    for (uint64_t i = 0;; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
        if (str1[i] == '\0' && str2[i] == '\0') {
            return true;
        }
    }
}

INLINE static bool streqn(const char *str1, const char *str2, uint64_t n) {
    for (uint64_t i = 0; i < n; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

INLINE static uint64_t strlen(const char *str,
                              uint64_t lengthLimit = (uint64_t)(-1)) {
    for (uint64_t i = 0; i < lengthLimit; ++i) {
        if (str[i] == 0) {
            return i;
        }
    }
    return lengthLimit;
}

INLINE static char *strdup(const char *str) {
    uint64_t len = strlen(str);
    char *result = new char[len + 1];
    memcpy(result, str, len);
    result[len] = '\0';
    return result;
}

struct CPUIDInfo {
    uint32_t leaf;
    uint32_t eax, ebx, ecx, edx;
};

INLINE CPUIDInfo cpuid(uint32_t leaf) {
    CPUIDInfo result;
    result.leaf = leaf;
    __get_cpuid(leaf, &result.eax, &result.ebx, &result.ecx, &result.edx);
    return result;
}

INLINE uint64_t strhash(const char *str) {
    uint64_t hash = 5381;
    for (uint64_t i = 0; ((const uint8_t *)str)[i] != '\0'; ++i) {
        hash = ((hash << 5) + hash) + ((const uint8_t *)str)[i];
    }
    return hash;
}

INLINE void hexdump(void *loc, uint64_t size) {
    uint8_t *p = (uint8_t *)loc;
    for (uint64_t i = 0; i < size; ++i) {
        if (i % 50 == 0) {
            core::log("\n");
        }
        static char digits[] = "0123456789ABCDEF";
        uint8_t atloc = p[i];
        uint8_t div, mod;
        div = atloc / 16;
        mod = atloc % 16;
        char arr[3];
        arr[0] = digits[div];
        arr[1] = digits[mod];
        arr[2] = '\0';
        core::log("%s", arr);
    }
}

INLINE uint64_t toCanonical(uint64_t addr) {
    return ((addr & (1ULL << 47)) == 0) ? (addr & ((1ULL << 48) - 1))
                                        : (addr | ~((1ULL << 48) - 1));
}

#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

extern "C" uint64_t getPageTable();
extern "C" uint64_t getFlags();
extern "C" uint64_t getCR2();
extern "C" uint64_t getFS();
extern "C" uint64_t getGS();
extern "C" uint64_t getCS();
extern "C" uint64_t getDS();
extern "C" uint64_t getES();
extern "C" uint64_t getSS();

#endif