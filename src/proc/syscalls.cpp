#include <proc/proc.hpp>
#include <proc/syscalls.hpp>

namespace proc {
    [[noreturn]] void sysExit() { proc::ProcessManager::exit(); }
}; // namespace proc