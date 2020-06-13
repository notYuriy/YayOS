#ifndef __DYNARRAY_HPP_INCLUDED__
#define __DYNARRAY_HPP_INCLUDED__

#include <core/cpprt.hpp>
#include <utils.hpp>

namespace core {

    template <class T> class DynArray {
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
        INLINE bool relocate(uint64_t newCapacity) {
            T *newarr = new T[newCapacity];
            if (newarr == nullptr) {
                return false;
            }
            memcpy(newarr, m_data, m_size);
            delete m_data;
            m_data = newarr;
            m_capacity = newCapacity;
            return true;
        }
        INLINE void init() {
            m_data = new T[16];
            m_size = 0;
            m_capacity = 16;
            m_disposed = false;
        }

    public:
        INLINE DynArray() { init(); }
        INLINE DynArray(DynArray &&other) {
            m_data = other.m_data;
            m_capacity = other.m_capacity;
            m_disposed = other.m_disposed;
            m_size = other.m_size;
        }
        INLINE bool pushBack(T elem) {
            m_data[m_size++] = elem;
            if (m_size == m_capacity) {
                uint64_t newCapacity = calculcateCapacity(m_size);
                if (!relocate(newCapacity)) {
                    m_size--;
                    return false;
                }
            }
            return true;
        }
        INLINE void popBack() {
            m_size--;
            if (m_capacity != calculcateCapacity(m_size)) {
                uint64_t newCapacity = calculcateCapacity(m_size);
                relocate(newCapacity);
            }
        }
        INLINE T &operator[](uint64_t index) { return at(index); }
        INLINE uint64_t size() { return m_size; }
        INLINE void dispose() {
            if (!m_disposed) {
                delete m_data;
            }
        }
        INLINE void clear() {
            delete m_data;
            init();
        }
        INLINE T &at(uint64_t index) {
            if (UNLIKELY(index >= m_size)) {
                panic("Vec: array overflow");
            }
            return m_data[index];
        }
        INLINE ~DynArray() { dispose(); }
    };

}; // namespace core

#endif