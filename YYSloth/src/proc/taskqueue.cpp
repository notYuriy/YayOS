#include <proc/intlock.hpp>
#include <proc/taskqueue.hpp>

namespace proc {

    void ProcessQueue::init() {
        m_taskhead = nullptr;
        m_tasktail = nullptr;
    }

    void ProcessQueue::sleep() {
        disableInterrupts();
        Process *current = ProcessManager::getRunningProcess();
        current->prev = nullptr;
        if (m_taskhead == nullptr) {
            m_taskhead = current;
            m_tasktail = current;
        } else {
            m_tasktail->prev = current;
            m_tasktail = current;
        }
        ProcessManager::suspendFromRunList(current->pid);
    }

    bool ProcessQueue::awake() {
        disableInterrupts();
        Process *toAwake = m_taskhead;
        if (toAwake != nullptr) {
            m_taskhead = toAwake->prev;
            ProcessManager::addToRunList(toAwake->pid);
            enableInterrupts();
            return true;
        } else {
            enableInterrupts();
            return false;
        }
    }

    bool ProcessQueue::empty() { return m_taskhead == nullptr; }

}; // namespace proc