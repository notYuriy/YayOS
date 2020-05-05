#include <proc/mutex.hpp>
#include <proc/sched.hpp>

namespace proc {

    Mutex::Mutex() : m_lock(1) {}
    void Mutex::lock() {
        if (Scheduler::isInitilaized()) {
            m_lock.acquire();
        }
    }
    void Mutex::unlock() {
        if (Scheduler::isInitilaized()) {
            m_lock.release();
        }
    }
    bool Mutex::someoneWaiting() { return m_lock.someoneWaiting(); }

}; // namespace proc