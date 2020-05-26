#include <proc/proc.hpp>
#include <proc/syscalls.hpp>

namespace proc {
    [[noreturn]] void sysExit() { proc::ProcessManager::exit(); }
    int64_t sysHelloWorld() {
        core::log("Hello world!\n\r");
        return 1;
    }
}; // namespace proc