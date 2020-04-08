#ifndef __PROC_HPP_INCLUDED__
#define __PROC_HPP_INLCUDED__

#include <spinlock.hpp>
#include <state.hpp>
#include <timer.hpp>
#include <utils.hpp>

namespace proc {

    typedef Uint64 Pid;
    const Pid pidCount = 65536;

    struct Process {
        ProcessState state;
        Pid pid;
        Process* next;
        Process* prev;
        Uint64 padding[3];
    };

    static_assert(sizeof(Process) % 64 == 0);

    typedef void (*EntryPoint)();

    class ProcessManager {
        static bool initialized;
        static Process* schedListHead;
        static Process* processData;
        static Uint64* pidBitmap;
        static Uint64 pidBitmapSize;
        static Uint64 lastCheckedIndex;
        static lock::Spinlock lock;
        static Process* execAlternative;

        static Pid pidAlloc();
        static void freePid(Pid pid);

    public:
        static drivers::Timer* timer;
        
        INLINE static bool isInitilaized() { return initialized; }
        static void schedule(SchedulerIntFrame* frame);
        static void init(drivers::Timer* timer);
        static Pid newProc(Process *proc);
    };

}; // namespace proc

#endif