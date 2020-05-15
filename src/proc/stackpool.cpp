#include <memory/kvmmngr.hpp>
#include <proc/intlock.hpp>
#include <proc/stackpool.hpp>

namespace proc {
    Stack *StackPool::m_head;

    void StackPool::init() { m_head = nullptr; }

    uint64_t StackPool::getNewStack() {
        disableInterrupts();
        Stack *result = m_head;
        if (result == nullptr) {
            enableInterrupts();
            return memory::KernelVirtualAllocator::getMapping(
                0x10000, 0, memory::DEFAULT_KERNEL_FLAGS);
        }
        m_head = m_head->next;
        enableInterrupts();
        return result->toUint64();
    }

    void StackPool::pushStack(uint64_t rsp) {
        Stack *stack = Stack::fromUint64(rsp);
        stack->next = m_head;
        m_head = stack;
    }

    bool StackPool::freeStack() {
        disableInterrupts();
        bool result = true;
        if (m_head == nullptr) {
            result = false;
            enableInterrupts();
        } else {
            Stack *stack = m_head;
            m_head = m_head->next;
            enableInterrupts();
            memory::KernelVirtualAllocator::unmapAt(stack->toUint64(), 0x10000);
        }
        return result;
    }

    bool StackPool::freeStackAvailable() { return m_head != nullptr; }

}; // namespace proc