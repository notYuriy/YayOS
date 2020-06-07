#ifndef __MSECURITY_HPP__
#define __MSECURITY_HPP__

#include <memory/vmmap.hpp>
#include <utils.hpp>

namespace memory {
    bool virtualRangeConditionCheck(vaddr_t start, uint64_t size, bool isUser,
                                    bool isWritable, bool isCode);
}; // namespace memory

#endif