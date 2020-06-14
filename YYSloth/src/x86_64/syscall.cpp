#include <proc/userapi.hpp>
#include <x86_64/interrupts.hpp>
#include <x86_64/syscall.hpp>

extern "C" {
uint64_t syscallHandlerTable[x86_64::SYSCALL_MAX] = {0};
}

namespace x86_64 {
    extern "C" void syscallHandler();
    void SyscallTable::loadSystemCallsToTable() {
        syscallHandlerTable[0] = (uint64_t)proc::YY_ExitProcess;
        syscallHandlerTable[1] = (uint64_t)proc::YY_DuplicateProcess;
        syscallHandlerTable[2] = (uint64_t)proc::YY_ConsoleWrite;
        syscallHandlerTable[3] = (uint64_t)proc::YY_GetSystemInfo;
        syscallHandlerTable[4] = (uint64_t)proc::YY_Yield;
        syscallHandlerTable[5] = (uint64_t)proc::YY_VirtualAlloc;
        syscallHandlerTable[6] = (uint64_t)proc::YY_VirtualFree;
        syscallHandlerTable[7] = (uint64_t)proc::YY_QueryAPIInfo;
        syscallHandlerTable[8] = (uint64_t)proc::YY_CheckProcStatus;
        syscallHandlerTable[9] = (uint64_t)proc::YY_ExecuteBinary;
        syscallHandlerTable[10] = (uint64_t)proc::YY_OpenFile;
        syscallHandlerTable[11] = (uint64_t)proc::YY_ReadFile;
        syscallHandlerTable[12] = (uint64_t)proc::YY_WriteFile;
        syscallHandlerTable[13] = (uint64_t)proc::YY_GetFilePos;
        syscallHandlerTable[14] = (uint64_t)proc::YY_SetFilePos;
    }
    void SyscallTable::init() {
        memset(syscallHandlerTable, sizeof(syscallHandlerTable), 0);
        loadSystemCallsToTable();
        IDT::install(0x57, (IDTVector)syscallHandler, 3, true);
    }
}; // namespace x86_64