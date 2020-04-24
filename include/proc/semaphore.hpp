#ifndef __SEMAPHORE_HPP_INCLUDED__
#define __SEMAPHORE_HPP_INCLUDED__

#include <proc/taskqueue.hpp>
#include <utils.hpp>

namespace proc {
    struct Semaphore {
        uint64_t m_num;
        TaskQueue m_queue;
        bool m_aquired;

    public:
        void init(uint64_t max);
        void acquire();
        void release();
    };
}; // namespace proc

#endif