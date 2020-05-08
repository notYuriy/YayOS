#include <x86_64/msr.hpp>
#include <x86_64/syscall.hpp>

namespace x86_64 {
    extern "C" void syscallHandler();
    void SyscallTable::init() {
        wrmsr(MSR_STAR, (0x13LLU << 48) || (0x8LLU << 32));
        wrmsr(MSR_LSTAR, (uint64_t)syscallHandler);
        wrmsr(MSR_SFMASK, 0);
    }
}; // namespace x86_64