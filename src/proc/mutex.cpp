#include <proc/mutex.hpp>

namespace proc {

    void Mutex::init() { m_lock.init(1); }
    void Mutex::lock() { m_lock.acquire(); }
    void Mutex::unlock() { m_lock.release(); }

}; // namespace proc