#include <proc/syscalls.hpp>
#include <x86_64/interrupts.hpp>
#include <x86_64/syscall.hpp>

extern "C" {
uint64_t syscallHandlerTable[x86_64::SYSCALL_MAX] = {0};
}

namespace x86_64 {
    extern "C" void syscallHandler();
    void SyscallTable::loadSystemCallsToTable() {
        syscallHandlerTable[60] = (uint64_t)proc::sysExit;
        syscallHandlerTable[0] = (uint64_t)proc::sysHelloWorld;
    }
    void SyscallTable::init() {
        memset(syscallHandlerTable, sizeof(syscallHandlerTable), 0);
        loadSystemCallsToTable();
        IDT::install(0x57, (IDTVector)syscallHandler, 3, true);
    }
}; // namespace x86_64