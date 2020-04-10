#ifndef __SPINLOCK_HPP_INCLUDED__
#define __SPINLOCK_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {

    extern "C" void spinlockLock(Uint64* addr);
    extern "C" void spinlockUnlock(Uint64* addr);
    extern "C" bool spinlockTrylock(Uint64* addr);

    struct Spinlock {
        Uint64 lockValue;
        INLINE void lock() {
            spinlockLock(&lockValue);
        }
        INLINE void unlock() {
            spinlockUnlock(&lockValue);
        }
        INLINE bool trylock() {
            return spinlockTrylock(&lockValue);
        }
    };

};

#endif
