#ifndef __USERTYPES_HPP_INCLUDED__
#define __USERTYPES_HPP_INCLUDED__

#include <utils.hpp>

namespace fs {
    constexpr uint64_t SEEK_SET = 0;
    constexpr uint64_t SEEK_CUR = 1;
    constexpr uint64_t SEEK_END = 2;
    constexpr uint64_t NAME_MAX = 255;
    struct Dirent {
        uint64_t inodeNumber;
        uint16_t nameLength;
        char name[NAME_MAX + 1];
    };
}; // namespace fs

#endif