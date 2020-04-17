#ifndef __USER_TYPES_HPP_INCLUDED__
#define __USER_TYPES_HPP_INCLUDED__

#include <fs/fstypes.hpp>

namespace fs {

    struct FileStatistics {
        Device deviceNumber;
        INodeNumber inodeNumber;
        Mode mode;
        LinkCount linkCount;
        Device deviceType;
        Offset size;
        BlockSize blockSize;
        BlockCount blockCount;
    };

    struct DirectoryEntry {
        INodeNumber number;
        Mode mode;
        char name[];
    };

}; // namespace fs

#endif