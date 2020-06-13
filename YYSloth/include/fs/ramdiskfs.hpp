#ifndef __RAMDISK_FS_HPP_INCLUDED__
#define __RAMDISK_FS_HPP_INCLUDED__

#include <core/dynarray.hpp>
#include <fs/vfs.hpp>
#include <memory/kvmmngr.hpp>
#include <memory/memoryinfo.hpp>

namespace fs {

#pragma pack(1)
    struct V7TarHeader {
        char filename[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        char typeflag;
        char padding[355];
        uint8_t data[];
        uint64_t getSize();
        V7TarHeader *next();
        bool isDirectory();
    };
#pragma pack(0)

    static_assert(sizeof(V7TarHeader) == 512);

    struct RamdiskDirEntry {
        uint64_t inode, hash;
        const char *name;
    };

    struct RamdiskDirNode : INode {
        core::DynArray<RamdiskDirEntry> entries;
        virtual uint64_t lookup(const char *name);
        virtual IFile *open(bool writable);
    };

    struct RamdiskFileNode : INode {
        uint64_t fileSize;
        uint8_t *data;
        virtual uint64_t lookup(const char *name);
        virtual IFile *open(bool writable);
        RamdiskFileNode(V7TarHeader *header);
    };

    struct RamdiskFileView : IFile {
        RamdiskFileNode *node;
        uint64_t fileOffset;
        virtual int64_t read(int64_t size, uint8_t *buf);
        virtual int64_t write(int64_t size, const uint8_t *buf);
        virtual int64_t readdir(int64_t count, Dirent *buf);
        virtual int64_t lseek(int64_t offset, int64_t whence);
        virtual int64_t ltellg();
        virtual void flush();
    };

    struct RamdiskDirView : IFile {
        RamdiskDirNode *node;
        uint64_t currentEntryIndex;
        virtual int64_t read(int64_t size, uint8_t *buf);
        virtual int64_t write(int64_t size, const uint8_t *buf);
        virtual int64_t readdir(int64_t count, Dirent *buf);
        virtual int64_t lseek(int64_t offset, int64_t whence);
        virtual int64_t ltellg();
        virtual void flush();
    };

    struct RamdiskFsSuperblock : ISuperblock {
        memory::vaddr_t mappingBase;
        uint64_t mappingSize;
        core::DynArray<INode *> nodes;
        virtual uint64_t getRootNum();
        virtual INode *getNode(uint64_t num);
        virtual void dropNode(uint64_t num);
        virtual void mount();
        virtual void unmount();

    private:
        uint64_t parseHeader(V7TarHeader *header, V7TarHeader **lookahead);
    };
}; // namespace fs

#endif