#ifndef __LOW_PHYS_ALLOC_HPP_INCLUDED__
#define __LOW_PHYS_ALLOC_HPP_INCLUDED__

#include <mm/memoryinfo.hpp>
#include <mm/physbase.hpp>
#include <utils.hpp>

namespace memory {
    class LowMemPhysAllocator {
        static bool m_initialized;

    public:
        static void init();
        INLINE static bool isInitialized() { return m_initialized; }
    };
} // namespace memory

#endif