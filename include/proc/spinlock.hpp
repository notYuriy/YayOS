#ifndef __SPINLOCK_HPP_INCLUDED__
#define __SPINLOCK_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {

    extern "C" void spinlockLock(uint64_t *addr);
    extern "C" void spinlockUnlock(uint64_t *addr);
    extern "C" bool spinlockTrylock(uint64_t *addr);

    struct Spinlock {
        uint64_t m_lockValue;

        INLINE void lock() { spinlockLock(&m_lockValue); }
        INLINE void unlock() { spinlockUnlock(&m_lockValue); }
        INLINE bool trylock() { return spinlockTrylock(&m_lockValue); }
    };

}; // namespace proc

#endif
