#include <core/cpprt.hpp>
#include <memory/kvmmngr.hpp>
#include <proc/intlock.hpp>
#include <proc/mutex.hpp>
#include <x86_64/tss.hpp>

namespace proc {
    Process *ProcessManager::m_schedListHead;
    Process *ProcessManager::m_processData;
    uint64_t *ProcessManager::m_pidBitmap;
    uint64_t ProcessManager::m_pidBitmapSize;
    uint64_t ProcessManager::m_lastCheckedIndex;
    drivers::ITimer *ProcessManager::m_timer;
    bool ProcessManager::m_initialized;

    extern "C" void schedulerIntHandler();
    extern "C" void schedulerYield();

    extern "C" void scheduleUsingFrame(SchedulerIntFrame *frame) {
        ProcessManager::schedule(frame);
    }

    extern "C" void timerEOI() { ProcessManager::m_timer->onTerm(); }

    pid_t ProcessManager::newProcess() {
        for (; m_lastCheckedIndex < m_pidBitmapSize; ++m_lastCheckedIndex) {
            if (~m_pidBitmap[m_lastCheckedIndex] == 0) {
                continue;
            }
            uint64_t bit = bitScanForward(~m_pidBitmap[m_lastCheckedIndex]);
            m_pidBitmap[m_lastCheckedIndex] |= (1ULL << bit);
            return bit + m_lastCheckedIndex * 64;
        }
        return pidCount;
    }

    void ProcessManager::yield() { schedulerYield(); }

    void ProcessManager::schedule(SchedulerIntFrame *frame) {
        m_schedListHead->moveGsFromMSR();
        m_schedListHead->state.loadFromFrame(frame);
        m_schedListHead = m_schedListHead->next;
        m_schedListHead->state.loadToFrame(frame);
        x86_64::TSS::setKernelStack(m_schedListHead->interruptsStackTop);
        m_schedListHead->moveGsToMSR();
    }

    void ProcessManager::freePid(pid_t pid) {
        uint64_t index = pid / 64;
        if (index < m_lastCheckedIndex) {
            m_lastCheckedIndex = index;
        }
        m_pidBitmap[index] &= ~(1ULL << (pid % 64));
    }

    bool Process::setup() {
        interruptsStackBase = memory::KernelVirtualAllocator::getMapping(
            0x10000, 0, memory::DEFAULT_KERNEL_FLAGS);
        if (interruptsStackBase == 0) {
            return false;
        }
        syscallStackBase = memory::KernelVirtualAllocator::getMapping(
            0x10000, 0, memory::DEFAULT_KERNEL_FLAGS);
        if (syscallStackBase == 0) {
            memory::KernelVirtualAllocator::unmapAt(interruptsStackBase,
                                                    0x10000);
            return false;
        }
        usralloc = memory::newUserVirtualAllocator();
        if (usralloc == nullptr) {
            memory::KernelVirtualAllocator::unmapAt(interruptsStackBase,
                                                    0x10000);
            memory::KernelVirtualAllocator::unmapAt(syscallStackBase, 0x10000);
            return false;
        }
        interruptsStackSize = syscallStackSize = 0x10000;
        interruptsStackTop = interruptsStackBase + 0x10000;
        syscallStackTop = syscallStackBase + 0x10000;
        savedUserRSP = 0;
        msrGS = (uint64_t)(this);
        return true;
    }

    void ProcessManager::init(drivers::ITimer *schedTimer) {
        m_timer = schedTimer;
        m_processData = (Process *)memory::KernelVirtualAllocator::getMapping(
            alignUp(sizeof(Process) * pidCount, 4096), 0,
            memory::DEFAULT_KERNEL_FLAGS);
        m_pidBitmap = new uint64_t[alignUp(pidCount, 64)];
        m_pidBitmapSize = alignUp(pidCount, 64) / 64;
        memset(m_pidBitmap, alignUp(pidCount, 64) / 64, 0);
        pid_t initPid = newProcess();
        Process *initProcess = &(m_processData[initPid]);
        initProcess->next = &m_processData[initPid];
        initProcess->prev = &m_processData[initPid];
        initProcess->pid = initPid;
        if (!initProcess->setup()) {
            panic("[ProcessManager] Failed to setup init process");
        }
        m_schedListHead = &m_processData[initPid];
        initProcess->moveGsToMSR();
        schedTimer->setCallback((x86_64::IDTVector)schedulerIntHandler);
    }

    void ProcessManager::addToRunList(pid_t proc) {
        disableInterrupts();
        Process *head = m_schedListHead;
        Process *prev = m_schedListHead->prev;
        m_processData[proc].next = head;
        m_processData[proc].prev = prev;
        head->prev = &(m_processData[proc]);
        prev->next = &(m_processData[proc]);
        enableInterrupts();
    }

    void ProcessManager::suspendFromRunList(pid_t pid) {
        disableInterrupts();
        Process *proc = &(m_processData[pid]);
        Process *prev = proc->prev;
        proc->next->prev = prev;
        prev->next = proc->next;
        if (pid == m_schedListHead->pid) {
            proc::ProcessManager::yield();
        }
        enableInterrupts();
    }

    void Process::cleanup() {
        delete usralloc;
        memory::KernelVirtualAllocator::unmapAt(syscallStackBase, 0x10000);
        memory::KernelVirtualAllocator::unmapAt(interruptsStackBase, 0x10000);
    }

    void ProcessManager::kill(pid_t pid) {
        Process *proc = getProcessData(pid);
        if (pid == m_schedListHead->pid) {
            proc->cleanup();
            suspendFromRunList(pid);
        } else {
            suspendFromRunList(pid);
            proc->cleanup();
        }
    }

    void ProcessManager::exit() { kill(m_schedListHead->pid); }

    Process *ProcessManager::getProcessData(pid_t pid) {
        return &(m_processData[pid]);
    }

    Process *ProcessManager::getRunningProcess() { return m_schedListHead; }

}; // namespace proc