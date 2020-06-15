#ifndef __DEVFS_HPP_INCLUDED__
#define __DEVFS_HPP_INCLUDED__

#include <core/dynarray.hpp>
#include <fs/vfs.hpp>
#include <proc/mutex.hpp>

namespace fs {

    struct DevFSRootFile : IFile {
        struct DevFSSuperblock *sb;
        uint64_t currentIndex;
        virtual int64_t readdir(int64_t count, Dirent *buf);
    };

    struct DevFSRootNode : INode {
        virtual uint64_t lookup(const char *name);
        virtual IFile *open(bool writable);
        virtual ~DevFSRootNode();
    };

    struct DevINode : INode {
        virtual IFile *open(bool writable) = 0;
        virtual ~DevINode();
    };

    struct DeviceEntry {
        DevINode *node;
        char name[NAME_MAX + 1];
    };

    struct DevFSSuperblock : ISuperblock {
        proc::Mutex rootMutex;
        core::DynArray<DeviceEntry> devices;
        DevFSRootNode node;
        virtual uint64_t getRootNum();
        virtual INode *getNode(uint64_t num);
        virtual void dropNode(uint64_t num);
        virtual void mount();
        virtual void unmount();
        bool registerDevice(const char *name, DevINode *node);
    };

}; // namespace fs

#endif