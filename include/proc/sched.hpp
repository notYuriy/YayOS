#ifndef __SCHED_HPP_INCLUDED__
#define __SCHED_HPP_INCLUDED__

#include <drivers/timer/timer.hpp>
#include <proc/spinlock.hpp>
#include <proc/state.hpp>
#include <utils.hpp>

namespace proc {

    typedef uint64_t pid_t;
    const pid_t pidCount = 65536;
    typedef void (*EntryPoint)();

    struct Task {
        TaskState state;
        pid_t pid;
        Task *next;
        Task *prev;
        uint64_t padding[4];
    };

    static_assert(sizeof(Task) % 64 == 0);

    class Scheduler {
        static Task *m_schedListHead;
        static Task *m_processData;
        static uint64_t *m_pidBitmap;
        static uint64_t m_pidBitmapSize;
        static uint64_t m_lastCheckedIndex;
        static bool m_initialized;

    public:
        static drivers::ITimer *m_timer;

        INLINE static bool isInitilaized() { return m_initialized; }
        static void schedule(SchedulerIntFrame *frame);
        static void yield();
        static void init(drivers::ITimer *timer);
        static bool addToRunList(Task *task);
        static bool suspendFromRunList(Task *task);
        static Task *getRunningTask();
    };

}; // namespace proc

#endif