#ifndef __VFS_HPP_INCLUDED__
#define __VFS_HPP_INCLUDED__

#include <fs/fstypes.hpp>
#include <fs/usertypes.hpp>
#include <proc/spinlock.hpp>
#include <utils.hpp>

namespace fs {

    struct IFilesystemInstance {
        struct IFilesystemType* type;
        IFilesystemInstance* left;
        IFilesystemInstance* right;
        virtual struct INode* get(INodeNumber number);
        virtual void drop(INodeNumber number);
        virtual INodeNumber alloc();
        virtual void link(INodeNumber number);
        virtual void unlink(INodeNumber number);
        virtual void dirty(INodeNumber number);
        virtual void write(INodeNumber);
        virtual void sync();
        virtual struct INode* getRoot();
    };

    struct INode {
        INodeNumber number;
        IFilesystemInstance* sb;
        virtual struct IFile* open(const char* perm);
        virtual Int64 stat(FileStatistics* entry);
        virtual INodeNumber lookup(const char* path);
        virtual Int64 unlink(const char* path);
        virtual Int64 link(const char* path, INode* node);
    };

    struct IFile {
        struct FileTreeNode* treeNode;
        virtual Int64 read(Int64 count, const char* buf);
        virtual Int64 write(Int64 count, const char* buf);
        virtual Int64 seek(Int64 count, const char* buf);
        Int64 fstat(FileStatistics* stats);
        virtual Int64 readdir(Int64 count, DirectoryEntry* buf);
        virtual void finalize();
        void close();
    };

    struct FileTreeListNode {
        char* name;
        struct FileTreeNode* node;
        Uint64 hash;
        FileTreeListNode* prev;
        FileTreeListNode* next;
    };

    struct FileTreeNode {
        Uint64 usedCount;
        INode* node;
        FileTreeListNode* fromParent;
        FileTreeNode* parent;
        FileTreeListNode* subdirs;

        FileTreeListNode* lookup(const char* name);
        void increaseRefCount();
        bool decreaseRefCount();
    };

    class VFS {
        static bool initialized;
        static IFilesystemInstance* fsList;
        static FileTreeNode* root;

    public:
        static void init(IFilesystemInstance* rootFS);
        static IFile* open(const char* path, const char* perm,
                           IFile* lookupStart = nullptr);
        INLINE static bool isInitialized() { return initialized; }
    };

}; // namespace fs

#endif