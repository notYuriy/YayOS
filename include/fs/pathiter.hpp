#ifndef __PATH_ITER_HPP_INCLUDED__
#define __PATH_ITER_HPP_INCLUDED__

#include <utils.hpp>

namespace fs {
    class PathIterator {
        char *m_pathcopy;
        uint64_t m_pos, m_endPos, m_basenamePos;

    public:
        INLINE PathIterator(const char *path) {
            m_pathcopy = strdup(path);
            m_endPos = strlen(path);
            m_basenamePos = 0;
            m_pos = 0;
            for (uint64_t i = 0; i < m_endPos; ++i) {
                if (m_pathcopy[i] == '/') {
                    m_pathcopy[i] = '\0';
                    m_basenamePos = i + 1;
                }
            }
        }
        INLINE const char *get() const { return m_pathcopy + m_pos; }
        INLINE bool atEnd(bool includeBasename = true) {
            return m_pos == (includeBasename ? m_endPos : m_basenamePos);
        }
        INLINE void next() {
            while ((m_pos < m_endPos) && (m_pathcopy[m_pos] != '\0')) {
                ++m_pos;
            };
            if (m_pos == m_endPos) {
                return;
            }
            m_pos++;
        }

        INLINE const char *getBasename() { return m_pathcopy + m_basenamePos; }
        INLINE ~PathIterator() { delete m_pathcopy; }
    };
}; // namespace fs

#endif