#ifndef __SYSCALL_HPP_INCLUDED__
#define __SYSCALL_HPP_INCLUDED__

namespace x86_64 {
    constexpr uint64_t SYSCALL_MAX = 1000;
    class SyscallTable {
        static void loadSystemCallsToTable();

    public:
        static void init();
    };
}; // namespace x86_64

#endif