#include <proc/syscalls.hpp>
#include <x86_64/msr.hpp>
#include <x86_64/syscall.hpp>

uint64_t syscallTable[x86_64::SYSCALL_MAX] = {0};

namespace x86_64 {
    extern "C" void syscallHandler();

    void SyscallTable::loadSystemCallsToTable() {
        syscallTable[60] = (uint64_t)proc::sysExit;
    }

    void SyscallTable::init() {
        wrmsr(MSR_STAR, (0x18LLU << 48) | (0x8LLU << 32));
        wrmsr(MSR_LSTAR, (uint64_t)syscallHandler);
        // enable interrupt on return
        wrmsr(MSR_SFMASK, (1LLU << 9));
        for (uint64_t i = 0; i < x86_64::SYSCALL_MAX; ++i) {
            syscallTable[i] = 0;
        }
        loadSystemCallsToTable();
    }

}; // namespace x86_64