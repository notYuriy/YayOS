#ifndef __MSR_HPP_INCLUDED__
#define __MSR_HPP_INCLUDED__

#include <utils.hpp>

namespace x86_64 {
    constexpr uint64_t MSR_STAR = 0xC0000081;
    constexpr uint64_t MSR_LSTAR = 0xC0000082;
    constexpr uint64_t MSR_CSTAR = 0xC0000083;
    constexpr uint64_t MSR_SFMASK = 0xC0000084;
    constexpr uint64_t MSR_GS = 0xC0000102;

    INLINE uint64_t rdmsr(uint64_t addr) {
        uint32_t high, low;
        asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(addr));
        return low | ((uint64_t)high << 32);
    }
    INLINE void wrmsr(uint64_t addr, uint64_t data) {
        uint32_t low = (uint32_t)data;
        uint32_t high = (uint32_t)(data >> 32);
        asm volatile("wrmsr" : : "c"(addr), "a"(low), "d"(high));
    }
}; // namespace x86_64

#endif