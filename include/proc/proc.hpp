#ifndef __PROC_HPP_INCLUDED__
#define __PROC_HPP_INCLUDED__

#include <drivers/timer/timer.hpp>
#include <mm/usrvmmngr.hpp>
#include <proc/state.hpp>

namespace proc {
    typedef uint64_t pid_t;
    constexpr pid_t pidCount = 65536;

    struct Process {
        TaskState state;
        Process *next, *prev;
        pid_t pid;
        uint64_t kernelStackTop, kernelStackSize, kernelStackBase;
        memory::UserVirtualAllocator *usralloc;
        void cleanup();
    };

    static_assert(sizeof(Process) % 64 == 0);

    class ProcessManager {
        static Process *m_schedListHead;
        static Process *m_processData;
        static uint64_t *m_pidBitmap;
        static uint64_t m_pidBitmapSize;
        static uint64_t m_lastCheckedIndex;
        static bool m_initialized;

        static void freePid(pid_t pid);
        static void cleanupPid(pid_t pid);

    public:
        static drivers::ITimer *m_timer;
        INLINE static bool isInitilaized() { return m_initialized; }
        static Process *getProcessData(pid_t pid);
        static Process *getRunningProcess();
        static void yield();
        static pid_t newProcess();
        static void init(drivers::ITimer *timer);
        static void schedule(SchedulerIntFrame *frame);
        static void addToRunList(pid_t pid);
        static void suspendFromRunList(pid_t pid);
        static void kill(pid_t pid);
        static void exit();
    };

} // namespace proc

#endif