#ifndef __UTILS_HPP_INCLUDED__
#define __UTILS_HPP_INCLUDED__

#include <attributes.hpp>
#include <core/cpprt.hpp>
#include <inttypes.hpp>
#include <core/log.hpp>
#include <stdarg.h>
#include <cpuid.h>

#define KB *1024
#define MB *1024 * 1024
#define GB *1024 * 1024 * 1024

INLINE static Uint64 alignUp(Uint64 value, Uint64 align) {
    return ((value + align - 1) / align) * align;
}

INLINE static Uint64 alignDown(Uint64 value, Uint64 align) {
    return (value / align) * align;
}

INLINE static void panic(const char* msg) {
    core::log("\u001b[31m");
    core::log(msg);
    while (1) {
        asm("pause");
    }
}

// Used for fast bitmap scanning. Source is taken from
// https://www.chessprogramming.org/BitScan#Bsf.2FBsr_x86-64_Timings
INLINE Uint8 bitScanForward(Uint64 x) {
    asm("bsfq %0, %0" : "=r"(x) : "0"(x));
    return (Uint8)x;
}

#define ALIGN_UP(x, align) (alignUp((Uint64)x, (Uint64)align))
#define ALIGN_DOWN(x, align) (alignUp((Uint64)x, (Uint64)align))

INLINE static void zeroPage(void* addr) {
    Uint64* p = (Uint64*)addr;
    for (Uint16 i = 0; i < 512; ++i) {
        p[i] = 0;
    }
}

INLINE static void memcpy(void* dst, void* src, Uint64 size) {
    for (Uint64 i = 0; i < size; ++i) {
        ((char*)dst)[i] = ((char*)src)[i];
    }
}

INLINE static void memset(void* dst, Uint64 size, Uint8 fill) {
    for (Uint64 i = 0; i < size; ++i) {
        ((char*)dst)[i] = fill;
    }
}

INLINE static bool streqn(const char* str1, const char* str2, Uint64 n) {
    for (Uint64 i = 0; i < n; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

struct CPUIDInfo {
    Uint32 leaf;
    Uint32 eax, ebx, ecx, edx;
};

INLINE CPUIDInfo cpuid(Uint32 leaf) {
    CPUIDInfo result;
    result.leaf = leaf;
    __get_cpuid(leaf, &result.eax, &result.ebx, &result.ecx, &result.edx);
    return result;
}

#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

extern "C" Uint64 getPageTable();
extern "C" Uint64 getFlags();
extern "C" Uint64 getFS();
extern "C" Uint64 getGS();
extern "C" Uint64 getCS();
extern "C" Uint64 getDS();
extern "C" Uint64 getES();
extern "C" Uint64 getSS();

#endif