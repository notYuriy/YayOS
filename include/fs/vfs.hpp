#ifndef __VFS_HPP_INCLUDED__
#define __VFS_HPP_INCLUDED__

#include <proc/mutex.hpp>
#include <utils.hpp>

namespace fs {

    struct Dirent {
        uint64_t inodeNumber;
        uint16_t nameLength;
        char name[];
    };

    struct IFile {
        struct DEntry *entry;
        virtual int64_t read(UNUSED int64_t size, UNUSED char *buf) {
            return -1;
        }
        virtual int64_t readdir(UNUSED int64_t count, UNUSED Dirent *buf) {
            return -1;
        }
    };

    struct INode {
        struct ISuperblock *sb;
        uint64_t num;
        virtual uint64_t lookup(UNUSED const char *name) { return 0; }
        virtual IFile *open(UNUSED int perm) { return nullptr; }
    };

    struct ISuperblock {
        virtual uint64_t getRootNum() = 0;
        virtual INode *getNode(uint64_t num) = 0;
        virtual void dropNode(uint64_t num) = 0;
        virtual void mount() = 0;
        virtual void unmount() = 0;
    };

    struct DEntry {
        DEntry *par, *next, *prev, *chld;
        proc::Mutex mutex;
        // usedCount =
        // number of threads traversing fs in this DEntry +
        // number of opened file descriptors here
        // number of children nodes
        uint64_t usedCount, nameHash;
        ISuperblock *sb;
        INode *node;
        char *name;

        static DEntry *createNode(const char *name);
        DEntry *createChildNode(const char *name);
        DEntry *softLookup(const char *name);
        DEntry *hardLookup(const char *name);
        DEntry *goToParent();
        DEntry *goToChild(const char *name);
        void incrementUsedCount();
        void decrementUsedCount();
        void dispose();
        void cut();
        bool drop();
        void dropRec();
    };
}; // namespace fs

#endif