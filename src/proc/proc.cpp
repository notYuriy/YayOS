#include <kheap.hpp>
#include <kvmmngr.hpp>
#include <proc.hpp>

extern "C" void timerEOI() {
    proc::ProcessManager::timer->onTerm();
}

namespace proc {

    bool ProcessManager::initialized;
    Process* ProcessManager::schedListHead;
    Process* ProcessManager::processData;
    Uint64* ProcessManager::pidBitmap;
    Uint64 ProcessManager::lastCheckedIndex;
    lock::Spinlock ProcessManager::lock;
    Uint64 ProcessManager::pidBitmapSize;
    Process* ProcessManager::execAlternative;
    drivers::Timer* ProcessManager::timer;

    extern "C" void schedulerIntHandler();
    extern "C" void scheduleUsingFrame(SchedulerIntFrame* frame) {
        ProcessManager::schedule(frame);
    }

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

    void ProcessManager::schedule(SchedulerIntFrame* frame) {
        kprintf("Switch\n\r");
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

    void ProcessManager::init(drivers::Timer* schedTimer) {
        if (!memory::KernelHeap::isInitialized()) {
            return;
        }
        timer = schedTimer;
        processData = (Process*)memory::KernelVirtualAllocator::getMapping(
            alignUp(sizeof(Process) * pidCount, 4096), 0, memory::defaultKernelFlags);
        pidBitmap = new Uint64[alignUp(pidCount, 64)];
        pidBitmapSize = alignUp(pidCount, 64) / 64;
        memset(pidBitmap, alignUp(pidCount, 64) / 64, 0);
        Pid initPid = pidAlloc();
        processData[initPid].next = &processData[initPid];
        processData[initPid].prev = &processData[initPid];
        schedListHead = &processData[initPid];
        interrupts::IDT::install(32, (interrupts::IDTVector)schedulerIntHandler);
        lock.lockValue = 0;
    }

}; // namespace proc