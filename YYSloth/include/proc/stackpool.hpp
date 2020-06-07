#ifndef __STACK_POOL_HPP_INCLUDED__
#define __STACK_POOL_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {
    struct Stack {
        Stack *next;
        INLINE static Stack *fromUint64(uint64_t rsp) { return (Stack *)rsp; }
        INLINE uint64_t toUint64() { return (uint64_t)(this); }
    };

    class StackPool {
        static Stack *m_head;

    public:
        static void init();
        static bool freeStackAvailable();
        static uint64_t getNewStack();
        static void pushStack(uint64_t rsp);
        static bool freeStack();
    };

} // namespace proc

#endif