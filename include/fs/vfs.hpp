#ifndef __VFS_HPP_INCLUDED__
#define __VFS_HPP_INCLUDED__

#include <fs/pathiter.hpp>
#include <proc/mutex.hpp>
#include <utils.hpp>

namespace fs {

    constexpr uint64_t NAME_MAX = 255;
    constexpr uint64_t SEEK_SET = 0;
    constexpr uint64_t SEEK_CUR = 1;
    constexpr uint64_t SEEK_END = 2;

    struct Dirent {
        uint64_t inodeNumber;
        uint16_t nameLength;
        char name[NAME_MAX + 1];
    };

    struct IFile {
        struct DEntry *entry;
        virtual int64_t read(int64_t size, uint8_t *buf) = 0;
        virtual int64_t write(int64_t size, const uint8_t *buf) = 0;
        virtual int64_t readdir(int64_t count, Dirent *buf) = 0;
        virtual int64_t lseek(int64_t offset, int64_t whence) = 0;
        virtual int64_t ltellg() = 0;
        virtual void flush() = 0;
        virtual ~IFile();
    };

    struct INode {
        struct ISuperblock *sb;
        uint64_t num;
        virtual uint64_t lookup(const char *name) = 0;
        virtual IFile *open(int perm) = 0;
        virtual ~INode();
    };

    struct ISuperblock {
        virtual uint64_t getRootNum() = 0;
        virtual INode *getNode(uint64_t num) = 0;
        virtual void dropNode(uint64_t num) = 0;
        virtual void mount() = 0;
        virtual void unmount() = 0;
        virtual ~ISuperblock();
    };

    struct DEntry {
        DEntry *par, *next, *prev, *chld, *mnt;
        proc::Mutex mutex;
        bool isFilesystemRoot, isMountpoint;
        // usedCount =
        // number of threads traversing vfs in this DEntry +
        // number of opened file descriptors here +
        // number of children nodes
        uint64_t usedCount;
        uint64_t nameHash;
        ISuperblock *sb;
        INode *node;
        char *name;

        static DEntry *createNode(const char *name);
        DEntry *createChildNode(const char *name);
        DEntry *softLookup(const char *name);
        DEntry *hardLookup(const char *name);
        DEntry *goToParent();
        DEntry *goToChild(const char *name);
        // the last node remain in observed state (so it is not deleted)
        DEntry *walk(PathIterator *iter, bool resolveLast = true);
        void incrementUsedCount();
        void decrementUsedCount();
        void dispose();
        void cut();
        bool drop();
        void dropRec();
    };

    class VFS {
        // support for one fs for now
        static ISuperblock *m_rootFs;
        static DEntry *m_fsTree;

    public:
        static void init(ISuperblock *sb);
        static IFile *open(const char *path, int perm);
    };

}; // namespace fs

#endif