#ifndef __FS_TYPES_HPP_INCLUDED__
#define __FS_TYPES_HPP_INCLUDED__

#include <utils.hpp>

namespace fs {
    typedef Uint64 Device;
    typedef Uint64 INodeNumber;
    typedef Uint32 Mode;
    typedef Uint64 LinkCount;
    typedef Uint64 BlockSize;
    typedef Uint64 BlockCount;
    typedef Uint64 TimeStamp;
    typedef Uint64 Offset;
    typedef Uint8 Type;
}; // namespace fs

#endif