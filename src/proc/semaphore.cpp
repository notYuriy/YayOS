#include <core/log.hpp>
#include <proc/intlock.hpp>
#include <proc/semaphore.hpp>
#include <utils.hpp>

namespace proc {
    Semaphore::Semaphore(uint64_t max) {
        m_num = max;
        m_queue.init();
        m_aquired = false;
    }

    void Semaphore::acquire(uint64_t num) {
        disableInterrupts();
        core::log("aquire\n\r");
        if (m_aquired || m_num < num) {
            m_queue.sleep();
        }
        m_num -= num;
        m_aquired = true;
        enableInterrupts();
    }

    void Semaphore::release(uint64_t num) {
        disableInterrupts();
        core::log("release\n\r");
        m_num += num;
        if (!m_queue.awake()) {
            m_aquired = false;
        }
        enableInterrupts();
    }

    bool Semaphore::someoneWaiting() { return !m_queue.empty(); }

}; // namespace proc