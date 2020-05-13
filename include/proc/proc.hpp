#ifndef __PROC_HPP_INCLUDED__
#define __PROC_HPP_INCLUDED__

#include <drivers/timer/timer.hpp>
#include <memory/usrvmmngr.hpp>
#include <proc/state.hpp>
#include <x86_64/msr.hpp>

namespace proc {
    typedef uint64_t pid_t;
    constexpr pid_t pidCount = 65536;

    struct Process {
        TaskState state;
        Process *next, *prev;
        uint64_t syscallStackTop;
        uint64_t savedUserRSP;
        pid_t pid;
        uint64_t msrGS;
        uint64_t interruptsStackTop;
        uint64_t interruptsStackSize;
        uint64_t interruptsStackBase;
        uint64_t syscallStackSize;
        uint64_t syscallStackBase;
        memory::UserVirtualAllocator *usralloc;
        uint64_t pad[3];
        bool setup();
        void cleanup();
        INLINE void moveGsFromMSR() { msrGS = x86_64::rdmsr(x86_64::MSR_GS); }
        INLINE void moveGsToMSR() { x86_64::wrmsr(x86_64::MSR_GS, msrGS); }
    };

    static_assert(sizeof(Process) % 64 == 0);
    // this offsets are used in assembly code
    static_assert(offsetof(Process, syscallStackTop) == 728);
    static_assert(offsetof(Process, savedUserRSP) == 736);

    class ProcessManager {
        static Process *m_schedListHead;
        static Process *m_processData;
        static uint64_t *m_pidBitmap;
        static uint64_t m_pidBitmapSize;
        static uint64_t m_lastCheckedIndex;
        static Process *m_idleProcess;
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
        [[noreturn]] static void exit();
    };

} // namespace proc

#endif