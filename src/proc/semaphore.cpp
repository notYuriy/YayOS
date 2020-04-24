#include <core/log.hpp>
#include <proc/intlock.hpp>
#include <proc/semaphore.hpp>
#include <utils.hpp>

namespace proc {
    void Semaphore::init(uint64_t max) {
        m_num = max;
        m_queue.init();
        m_aquired = false;
    }

    void Semaphore::acquire() {
        disableInterrupts();
        if (m_aquired || m_num == 0) {
            m_queue.sleep();
        }
        m_num--;
        m_aquired = true;
        enableInterrupts();
    }

    void Semaphore::release() {
        disableInterrupts();
        m_num++;
        if (!m_queue.awake()) {
            m_aquired = false;
        }
        enableInterrupts();
    }

}; // namespace proc