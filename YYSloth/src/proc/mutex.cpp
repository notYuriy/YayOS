#include <proc/mutex.hpp>
#include <proc/proc.hpp>

namespace proc {

    Mutex::Mutex() : m_lock(1) {}
    void Mutex::lock() {
        if (ProcessManager::isInitilaized()) {
            m_lock.acquire();
        }
    }
    void Mutex::unlock() {
        if (ProcessManager::isInitilaized()) {
            m_lock.release();
        }
    }
    bool Mutex::someoneWaiting() { return m_lock.someoneWaiting(); }

}; // namespace proc