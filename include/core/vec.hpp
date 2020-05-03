#ifndef __VEC_HPP_INCLUDED__
#define __VEC_HPP_INCLUDED__

#include <utils.hpp>

namespace core {

    template <class T> class Vec {
        T *m_data;
        uint64_t m_size;
        uint64_t m_capacity;
        bool m_disposed;
        INLINE uint64_t calculcateCapacity(uint64_t size) {
            uint64_t result = 16;
            while (result <= size) {
                result *= 2;
            }
            return result;
        }
        INLINE void relocate(uint64_t newCapacity) {
            T *newarr = new T[newCapacity];
            memcpy(newarr, m_data, m_size);
            delete m_data;
            m_data = newarr;
            m_capacity = newCapacity;
        }

    public:
        INLINE Vec() {
            m_data = new T[16];
            m_size = 0;
            m_capacity = 16;
            m_disposed = false;
        }
        INLINE void pushBack(T elem) {
            m_data[m_size++] = elem;
            if (m_size == m_capacity) {
                uint64_t newCapacity = calculcateCapacity(m_size);
                relocate(newCapacity);
            }
        }
        INLINE void popBack() {
            m_size--;
            if (m_capacity != calculcateCapacity(m_size)) {
                uint64_t newCapacity = calculcateCapacity(m_size);
                relocate(newCapacity);
            }
        }
        INLINE T &operator[](uint64_t index) {
            if (UNLIKELY(index >= m_size)) {
                panic("Vec: array overflow");
            }
            return m_data[index];
        }
        INLINE uint64_t size() { return m_size; }
        INLINE void dispose() {
            if (!m_disposed) {
                delete m_data;
            }
        }
        INLINE ~Vec() { dispose(); }
    };

}; // namespace core

#endif