#ifndef __TASK_QUEUE_HPP_INCLUDED__
#define __TASK_QUEUE_HPP_INCLUDED__

#include <proc/proc.hpp>

namespace proc {
    class ProcessQueue {
        Process *m_taskhead;
        Process *m_tasktail;

    public:
        void init();
        void sleep();
        bool awake();
        bool empty();
    };
}; // namespace proc

#endif