#ifndef __VFS_HPP_INCLUDED__
#define __VFS_HPP_INCLUDED__

#include <fs/pathiter.hpp>
#include <fs/usertypes.hpp>
#include <proc/descriptor.hpp>
#include <proc/mutex.hpp>
#include <utils.hpp>

namespace fs {

    struct IFile : public proc::IDescriptor {
        struct DEntry *entry;
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
        static ISuperblock *m_rootFs[26];
        static DEntry *m_fsTrees[26];
        static proc::Mutex m_rootMutex;

    public:
        static void init();
        static bool mount(char letter, ISuperblock *sb);
        static IFile *open(const char *path, int perm);
    };

}; // namespace fs

#endif