#ifndef __TASK_QUEUE_HPP_INCLUDED__
#define __TASK_QUEUE_HPP_INCLUDED__

#include <proc/sched.hpp>

namespace proc {
    class TaskQueue {
        Task *m_taskhead;
        Task *m_tasktail;

    public:
        void init();
        void sleep();
        bool awake();
        bool empty();
    };
}; // namespace proc

#endif