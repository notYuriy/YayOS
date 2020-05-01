#include <proc/mutex.hpp>
#include <proc/sched.hpp>

namespace proc {

    void Mutex::init() { m_lock.init(1); }
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