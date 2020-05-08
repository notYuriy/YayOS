#ifndef __SYSCALL_HPP_INCLUDED__
#define __SYSCALL_HPP_INCLUDED__

namespace x86_64 {
    class SyscallTable {
    public:
        static void init();
    };
}; // namespace x86_64

#endif