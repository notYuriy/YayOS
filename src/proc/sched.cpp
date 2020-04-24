#include <mm/kheap.hpp>
#include <mm/kvmmngr.hpp>
#include <proc/intlock.hpp>
#include <proc/sched.hpp>

extern "C" void timerEOI() { proc::Scheduler::m_timer->onTerm(); }

namespace proc {

    bool Scheduler::m_initialized;
    Task *Scheduler::m_schedListHead;
    drivers::ITimer *Scheduler::m_timer;

    extern "C" void schedulerIntHandler();
    extern "C" void schedulerYield();

    extern "C" void scheduleUsingFrame(SchedulerIntFrame *frame) {
        Scheduler::schedule(frame);
    }

    extern "C" void procYield() { Scheduler::yield(); }

    void Scheduler::schedule(SchedulerIntFrame *frame) {
        m_schedListHead->state.loadFromFrame(frame);
        m_schedListHead = m_schedListHead->next;
        m_schedListHead->state.loadToFrame(frame);
    }

    void Scheduler::init(drivers::ITimer *schedTimer) {
        if (!memory::KernelHeap::isInitialized()) {
            return;
        }
        m_timer = schedTimer;
        Task *initTask = new Task;
        initTask->next = initTask;
        initTask->prev = initTask;
        m_schedListHead = initTask;
        m_timer->setCallback((core::IDTVector)schedulerIntHandler);
        m_initialized = true;
    }

    bool Scheduler::addToRunList(Task *proc) {
        disableInterrupts();
        Task *head = m_schedListHead;
        Task *prev = m_schedListHead->prev;
        proc->next = head;
        proc->prev = prev;
        head->prev = proc;
        prev->next = proc;
        enableInterrupts();
        return true;
    }

    void Scheduler::yield() { schedulerYield(); }

    bool Scheduler::suspendFromRunList(Task *task) {
        disableInterrupts();
        Task *prev = task->prev;
        task->next->prev = prev;
        prev->next = task->next;
        enableInterrupts();
        return true;
    }

    Task *Scheduler::getRunningTask() { return m_schedListHead; }

}; // namespace proc