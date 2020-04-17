#include <mm/kheap.hpp>
#include <mm/kvmmngr.hpp>
#include <proc/proc.hpp>

extern "C" void timerEOI() { proc::ProcessManager::timer->onTerm(); }

namespace proc {

    bool ProcessManager::initialized;
    Process *ProcessManager::schedListHead;
    Process *ProcessManager::processData;
    Uint64 *ProcessManager::pidBitmap;
    Uint64 ProcessManager::lastCheckedIndex;
    proc::Spinlock ProcessManager::modifierLock;
    Uint64 ProcessManager::pidBitmapSize;
    drivers::ITimer *ProcessManager::timer;
    Uint64 ProcessManager::yieldFlag;
    Uint64 ProcessManager::unlockSpinlock;

    extern "C" void schedulerIntHandler();
    extern "C" void schedulerYield();

    extern "C" void scheduleUsingFrame(SchedulerIntFrame *frame) {
        ProcessManager::schedule(frame);
    }

    extern "C" void setYieldFlag() { ProcessManager::yieldFlag = 1; }

    extern "C" void clearYieldFlag() { ProcessManager::yieldFlag = 0; }

    extern "C" void scheduleForIntUsingFrame(SchedulerIntFrame *frame) {
        if (ProcessManager::yieldFlag) {
            return;
        }
        scheduleUsingFrame(frame);
    }

    extern "C" void procYield() { ProcessManager::yield(); }

    Pid ProcessManager::pidAlloc() {
        for (; lastCheckedIndex < pidBitmapSize; ++lastCheckedIndex) {
            if (~pidBitmap[lastCheckedIndex] == 0) {
                continue;
            }
            Uint64 bit = bitScanForward(~pidBitmap[lastCheckedIndex]);
            pidBitmap[lastCheckedIndex] |= (1ULL << bit);
            return bit + lastCheckedIndex * 64;
        }
        return pidCount;
    }

    void ProcessManager::schedule(SchedulerIntFrame *frame) {
        // core::log("pid: %ull\n\r", schedListHead->pid);
        if (unlockSpinlock) {
            unlockSpinlock = false;
            modifierLock.unlock();
        }
        schedListHead->state.loadFromFrame(frame);
        schedListHead = schedListHead->next;
        schedListHead->state.loadToFrame(frame);
    }

    void ProcessManager::freePid(Pid pid) {
        Uint64 index = pid / 64;
        if (index < lastCheckedIndex) {
            lastCheckedIndex = index;
        }
        pidBitmap[index] &= ~(1ULL << (pid % 64));
    }

    void ProcessManager::init(drivers::ITimer *schedTimer) {
        if (!memory::KernelHeap::isInitialized()) {
            return;
        }
        timer = schedTimer;
        processData = (Process *)memory::KernelVirtualAllocator::getMapping(
            alignUp(sizeof(Process) * pidCount, 4096), 0,
            memory::defaultKernelFlags);
        pidBitmap = new Uint64[alignUp(pidCount, 64)];
        pidBitmapSize = alignUp(pidCount, 64) / 64;
        memset(pidBitmap, alignUp(pidCount, 64) / 64, 0);
        Pid initPid = pidAlloc();
        processData[initPid].next = &processData[initPid];
        processData[initPid].prev = &processData[initPid];
        processData[initPid].pid = initPid;
        schedListHead = &processData[initPid];
        timer->setCallback((core::IDTVector)schedulerIntHandler);
        modifierLock.lockValue = 0;
        unlockSpinlock = 1;
        yieldFlag = 0;
        initialized = true;
    }

    Process *ProcessManager::newProc() {
        modifierLock.lock();
        Pid pid = pidAlloc();
        if (pid == pidCount) {
            modifierLock.unlock();
            return nullptr;
        }
        modifierLock.unlock();
        processData[pid].pid = pid;
        return &processData[pid];
    }

    bool ProcessManager::addToRunList(Process *proc) {
        modifierLock.lock();
        Process *head = schedListHead;
        Process *prev = schedListHead->prev;
        proc->next = head;
        proc->prev = prev;
        head->prev = proc;
        prev->next = proc;
        modifierLock.unlock();
        return true;
    }

    void ProcessManager::yield() { schedulerYield(); }

    bool ProcessManager::suspendFromRunList(Pid pid) {
        modifierLock.lock();
        Process *proc = &processData[pid];
        Process *prev = proc->prev;
        freePid(pid);
        proc->next->prev = prev;
        unlockSpinlock = true;
        prev->next = proc->next;
        if (pid == schedListHead->pid) {
            while (1) {
                asm("pause");
            }
        }
        modifierLock.unlock();
        return true;
    }

}; // namespace proc