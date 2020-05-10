#include <x86_64/msr.hpp>
#include <x86_64/syscall.hpp>

uint64_t syscallTable[x86_64::SYSCALL_MAX] = {0, 0};

namespace x86_64 {
    extern "C" void syscallHandler();
    void SyscallTable::init() {
        wrmsr(MSR_STAR, (0x18LLU << 48) | (0x8LLU << 32));
        wrmsr(MSR_LSTAR, (uint64_t)syscallHandler);
        // enable interrupt on return
        wrmsr(MSR_SFMASK, (1LLU << 9));
    }
}; // namespace x86_64