#ifndef __VFS_HPP_INCLUDED__
#define __VFS_HPP_INCLUDED__

#include <fs/pathiter.hpp>
#include <fs/usertypes.hpp>
#include <proc/descriptor.hpp>
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

    class VFS {
        static ISuperblock *m_rootFs[26];
        static struct DEntry *m_fsTrees[26];

    public:
        static void init();
        static bool mount(char letter, ISuperblock *sb);
        static IFile *open(const char *path, int perm);
    };

}; // namespace fs

#endif