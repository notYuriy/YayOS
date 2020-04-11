#ifndef __VFS_HPP_INCLUDED__
#define __VFS_HPP_INCLUDED__

#include <utils.hpp>
#include <proc/spinlock.hpp>

namespace fs {

    struct IFileSystemInstance;
    struct IFileSystemNode;
    struct FileTreeNode;
    struct IFileView;
    struct DirectoryEntry;
    struct FileStatistics;

    struct IFileSystemInstance {
        IFileSystemNode* fsRoot;
    };

    struct IFileSystemNode {
        virtual Int64 open(IFileView** file, char* perm);
        virtual Int64 close(IFileView* file);
        virtual Int64 stat(FileStatistics* stat);
        virtual bool flush(FileTreeNode* node);
    };

    struct IFileView {
        IFileSystemNode* node;
        virtual Int64 read(Uint64 count, char* buf);
        virtual Int64 write(Uint64 count, char* buf);
        virtual Int64 seek(Uint64 pos, Int64 whence);
        virtual Int64 getdents(DirectoryEntry* entries, Uint64 count);
        virtual void flush();
        void openat(char* path, char* perm);
        void fstatat(char* path, FileStatistics* stat);
    };

    class VFS {
        IFileSystemInstance* rootFS;
    public:
        Int64 open(IFileView** file, char* path, char* perm);
        Int64 close(IFileView* view);
    };

}; // namespace fs

#endif