#ifndef __USER_MODE_HPP_INCLUDED__
#define __USER_MODE_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {
    extern "C" [[noreturn]] void jumpToUserMode(uint64_t entryPoint,
                                                uint64_t rsp);
};

#endif