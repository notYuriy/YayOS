#ifndef __MUTEX_HPP_INCLUDED__
#define __MUTEX_HPP_INCLUDED__

#include <proc/semaphore.hpp>

namespace proc {
    class Mutex {
        Semaphore m_lock;

    public:
        void init();
        void lock();
        void unlock();
        bool someoneWaiting();
    };
}; // namespace proc

#endif