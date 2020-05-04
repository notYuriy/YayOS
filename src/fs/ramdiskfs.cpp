#include <fs/ramdiskfs.hpp>
#include <mm/kheap.hpp>
#include <mm/kvmmngr.hpp>

namespace fs {
    uint64_t V7TarHeader::getSize() {
        uint64_t result = 0;
        uint64_t base = 1;
        for (uint64_t i = 11; i > 0; --i, base *= 8) {
            result += ((size[i - 1] - '0') * base);
        }
        return result;
    }

    V7TarHeader *V7TarHeader::next() {
        return (V7TarHeader *)alignUp(((uint64_t)this) + 512 + getSize(), 512);
    }

    bool V7TarHeader::isDirectory() { return typeflag == '5'; }
    INode *RamdiskFsSuperblock::getNode(uint64_t num) {
        if (num > nodes.size() || num == 0) {
            return nullptr;
        }
        return nodes[num - 1];
    }
    void RamdiskFsSuperblock::dropNode(UNUSED uint64_t num) {}
    uint64_t RamdiskFsSuperblock::getRootNum() { return nodes.size(); }
    uint64_t RamdiskFileNode::lookup(UNUSED const char *name) { return 0; }

    uint64_t RamdiskFsSuperblock::parseHeader(V7TarHeader *header,
                                              V7TarHeader **lookahead) {
        if (header->isDirectory()) {
            uint64_t namelen = strlen(header->filename);
            RamdiskDirNode *newNode = new RamdiskDirNode;
            V7TarHeader *cur = header->next();
            while (streqn(header->filename, cur->filename, namelen)) {
                const char *entryName = (cur->filename + namelen);
                char *cpy = strdup(entryName);
                uint64_t cpylen = strlen(cpy);
                if (cpy[cpylen - 1] == '/') {
                    cpy[cpylen - 1] = '\0';
                }
                uint64_t result = parseHeader(cur, &cur);
                RamdiskDirEntry entry = {result, strhash(cpy), cpy};
                newNode->entries.pushBack(entry);
            }
            *lookahead = cur;
            nodes.pushBack(newNode);
            return nodes.size();
        } else {
            INode *newNode = new RamdiskFileNode(header);
            nodes.pushBack(newNode);
            *lookahead = header->next();
            return nodes.size();
        }
    }

    void RamdiskFsSuperblock::mount() {
        mappingSize = memory::BootMemoryInfo::initrdLimit -
                      memory::BootMemoryInfo::initrdBase;
        mappingBase = memory::KernelVirtualAllocator::getMapping(
            mappingSize, memory::BootMemoryInfo::initrdBase,
            memory::DEFAULT_UNMANAGED_FLAGS);
        uint64_t startOffset = memory::BootMemoryInfo::initrdStart -
                               memory::BootMemoryInfo::initrdBase;
        memory::vaddr_t rdStart = mappingBase + startOffset;
        V7TarHeader *cur = (V7TarHeader *)rdStart;
        parseHeader(cur, &cur);
    }

    void RamdiskFsSuperblock::unmount() {
        for (uint64_t i = 0; i < nodes.size(); ++i) {
            delete nodes[i];
        }
        nodes.dispose();
        memory::KernelVirtualAllocator::unmapAt(mappingBase, mappingSize);
    }

    RamdiskFileNode::RamdiskFileNode(V7TarHeader *header) {
        fileSize = header->getSize();
        data = header->data;
    }

    uint64_t RamdiskDirNode::lookup(const char *name) {
        uint64_t hash = strhash(name);
        for (uint64_t i = 0; i < entries.size(); ++i) {
            if (entries[i].hash == hash && streq(entries[i].name, name)) {
                return entries[i].inode;
            }
        }
        return 0;
    }

    IFile *RamdiskDirNode::open(UNUSED int perm) {
        RamdiskDirView *result = new RamdiskDirView;
        if (result == nullptr) {
            return nullptr;
        }
        result->node = this;
        result->currentEntryIndex = 0;
        return result;
    }

    IFile *RamdiskFileNode::open(UNUSED int perm) {
        RamdiskFileView *result = new RamdiskFileView;
        if (result == nullptr) {
            return nullptr;
        }
        result->node = this;
        result->fileOffset = 0;
        return result;
    }

    int64_t RamdiskDirView::read(UNUSED int64_t size, UNUSED uint8_t *buf) {
        return -1;
    }
    int64_t RamdiskDirView::write(UNUSED int64_t size,
                                  UNUSED const uint8_t *buf) {
        return -1;
    }

    int64_t RamdiskDirView::readdir(int64_t count, Dirent *buf) {
        int64_t realCount = count;
        if (currentEntryIndex + realCount > node->entries.size()) {
            realCount = node->entries.size() - currentEntryIndex;
        }
        for (int64_t i = 0; i < realCount; ++i) {
            buf[i].inodeNumber = node->entries[currentEntryIndex + i].inode;
            buf[i].nameLength =
                strlen(node->entries[currentEntryIndex + i].name);
            memset(buf[i].name, NAME_MAX, '\0');
            memcpy(buf[i].name, node->entries[currentEntryIndex + i].name,
                   buf[i].nameLength);
        }
        currentEntryIndex += realCount;
        return realCount;
    }

    void RamdiskDirView::finalize() {}
    void RamdiskDirView::flush() {}

    int64_t RamdiskFileView::read(int64_t size, uint8_t *buf) {
        int64_t realSize = size;
        if (fileOffset + realSize > node->fileSize) {
            realSize = node->fileSize - fileOffset;
        }
        for (int64_t i = 0; i < realSize; ++i) {
            buf[i] = (node->data)[fileOffset + i];
        }
        fileOffset += realSize;
        return realSize;
    }

    int64_t RamdiskFileView::readdir(UNUSED int64_t count, UNUSED Dirent *buf) {
        return -1;
    }
    void RamdiskFileView::finalize() {}
    void RamdiskFileView::flush() {}
    int64_t RamdiskFileView::write(UNUSED int64_t size,
                                   UNUSED const uint8_t *buf) {
        return -1;
    }

    int64_t RamdiskFileView::lseek(int64_t offset, int64_t whence) {
        int64_t newpos;
        if (whence == SEEK_SET) {
            newpos = offset;
        } else if (whence == SEEK_CUR) {
            newpos = offset + fileOffset;
        } else {
            newpos = offset + node->fileSize;
        }
        if (newpos < 0) {
            return -1;
        } else if (newpos > (int64_t)(node->fileSize)) {
            return -1;
        }
        fileOffset = newpos;
        return newpos;
    }

    int64_t RamdiskFileView::ltellg() { return fileOffset; }
    int64_t RamdiskDirView::lseek(UNUSED int64_t offset,
                                  UNUSED int64_t whence) {
        return -1;
    }
    int64_t RamdiskDirView::ltellg() { return -1; }

}; // namespace fs