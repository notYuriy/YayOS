#ifndef __PROC_HPP_INCLUDED__
#define __PROC_HPP_INCLUDED__

#include <core/dynarray.hpp>
#include <drivers/timer/timer.hpp>
#include <memory/usrvmmngr.hpp>
#include <proc/descriptor.hpp>
#include <proc/state.hpp>
#include <x86_64/msr.hpp>

constexpr int64_t YY_NonRecoverableError = 3;

namespace proc {
    typedef uint64_t pid_t;
    constexpr pid_t PID_MAX = 65536;

    struct Process {
        TaskState state;
        Process *next, *prev;
        pid_t pid, ppid, dead;
        uint64_t kernelStackTop;
        uint64_t kernelStackSize;
        uint64_t kernelStackBase;
        memory::UserVirtualAllocator *usralloc;
        DescriptorTable table;
        int64_t returnCode;
        int64_t status;
        uint64_t pad[2];
        bool setup(bool createUserAllocator = true);
        void cleanup(bool freeTable = true);
    };

    static_assert(sizeof(Process) % 64 == 0);

    class ProcessManager {
        static Process *m_schedListHead;
        static Process *m_processData;
        static uint64_t *m_pidBitmap;
        static uint64_t m_pidBitmapSize;
        static uint64_t m_lastCheckedIndex;
        static Process *m_idleProcess;
        static bool m_initialized;

        static void cleanupPid(pid_t pid);

    public:
        static drivers::ITimer *m_timer;
        INLINE static bool isInitilaized() { return m_initialized; }
        static Process *getProcessData(pid_t pid);
        static Process *getRunningProcess();
        static void freePid(pid_t pid);
        static void yield();
        static pid_t newProcess();
        static void init(drivers::ITimer *timer);
        static void schedule(SchedulerIntFrame *frame);
        static void addToRunList(pid_t pid);
        static void suspendFromRunList(pid_t pid);
        static void kill(pid_t pid);
        [[noreturn]] static void exit(int64_t returnCode, int64_t status);
    };

} // namespace proc

#endif