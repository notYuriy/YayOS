#include <proc/intlock.hpp>
#include <proc/taskqueue.hpp>

namespace proc {

    void TaskQueue::init() {
        m_taskhead = nullptr;
        m_tasktail = nullptr;
    }

    void TaskQueue::sleep() {
        disableInterrupts();
        Task *current = Scheduler::getRunningTask();
        Scheduler::suspendFromRunList(current);
        current->prev = nullptr;
        if (m_taskhead == nullptr) {
            m_taskhead = current;
            m_tasktail = current;
        } else {
            m_tasktail->prev = current;
            m_tasktail = current;
        }
        Scheduler::yield();
    }

    bool TaskQueue::awake() {
        disableInterrupts();
        Task *toAwake = m_taskhead;
        if (toAwake != nullptr) {
            m_taskhead = toAwake->prev;
            Scheduler::addToRunList(toAwake);
            enableInterrupts();
            return true;
        } else {
            enableInterrupts();
            return false;
        }
    }

    bool TaskQueue::empty() { return m_taskhead == nullptr; }

}; // namespace proc