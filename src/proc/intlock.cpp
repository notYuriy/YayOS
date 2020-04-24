#include <proc/intlock.hpp>

#include <utils.hpp>

namespace proc {
    uint64_t intsCount;
    void disableInterrupts() {
        asm __volatile__("cli" :::);
        intsCount++;
    }
    void enableInterrupts() {
        if (intsCount > 0) {
            intsCount--;
        }
        if (intsCount == 0) {
            asm __volatile__("sti" :::);
        }
    }
    extern "C" void zeroIntLevel() { intsCount = 0; }
}; // namespace proc