#include <fs/devfs.hpp>

namespace fs {
    int64_t DevFSRootFile::read(UNUSED int64_t size, UNUSED uint8_t *buf) {
        return -1;
    }

    int64_t DevFSRootFile::write(UNUSED int64_t size,
                                 UNUSED const uint8_t *buf) {
        return -1;
    }

    int64_t DevFSRootFile::readdir(int64_t count, Dirent *buf) {
        sb->rootMutex.lock();
        for (int64_t i = 0; i < count;) {
            if (currentIndex >= sb->devices.size()) {
                sb->rootMutex.unlock();
                return i;
            }
            if (sb->devices[currentIndex].node == nullptr) {
                currentIndex++;
                continue;
            }
            buf->inodeNumber = 2 + currentIndex;
            memset(buf->name, NAME_MAX + 1, '\0');
            buf->nameLength = strlen(sb->devices[currentIndex].name);
            memcpy(buf->name, sb->devices[currentIndex].name, buf->nameLength);
            ++currentIndex;
            ++i;
        }
        sb->rootMutex.unlock();
        return count;
    }

    int64_t DevFSRootFile::lseek(UNUSED int64_t offset, UNUSED int64_t whence) {
        return -1;
    }

    int64_t DevFSRootFile::ltellg() { return -1; }

    void DevFSRootFile::flush() {}

    uint64_t DevFSRootNode::lookup(const char *name) {
        DevFSSuperblock *devsb = (DevFSSuperblock *)sb;
        devsb->rootMutex.lock();
        for (uint64_t i = 0; i < devsb->devices.size(); ++i) {
            if (streq(devsb->devices[i].name, name)) {
                if (devsb->devices[i].node == nullptr) {
                    devsb->rootMutex.unlock();
                    return 0;
                }
                devsb->rootMutex.unlock();
                return 2 + i;
            }
        }
        devsb->rootMutex.unlock();
        return 0;
    }

    IFile *DevFSRootNode::open(UNUSED bool writable) {
        DevFSRootFile *result = new DevFSRootFile;
        result->currentIndex = 0;
        result->sb = (DevFSSuperblock *)sb;
        return result;
    }

    DevFSRootNode::~DevFSRootNode() {}

    uint64_t DevFSSuperblock::getRootNum() { return 1; }

    INode *DevFSSuperblock::getNode(uint64_t num) {
        if (num == 1) {
            return &node;
        }
        if (!(num >= 2 && num < (2 + devices.size()))) {
            return nullptr;
        }
        return devices[num - 2].node;
    }

    void DevFSSuperblock::dropNode(UNUSED uint64_t num) {}

    void DevFSSuperblock::mount() {
        node.sb = this;
        node.num = 1;
    }

    void DevFSSuperblock::unmount() {}

    bool DevFSSuperblock::registerDevice(const char *name, DevINode *node) {
        rootMutex.lock();
        for (int64_t i = 0; i < (int64_t)(devices.size()); ++i) {
            if (devices[i].node == nullptr) {
                devices[i].node = node;
                memset(devices[i].name, NAME_MAX + 1, '\0');
                memcpy(devices[i].name, name, strlen(name));
                rootMutex.unlock();
                return true;
            }
        }
        if (!devices.pushBack(DeviceEntry())) {
            rootMutex.unlock();
            return false;
        } else {
            devices[devices.size() - 1].node = node;
            memset(devices[devices.size() - 1].name, NAME_MAX + 1, '\0');
            memcpy(devices[devices.size() - 1].name, name, strlen(name));
        }
        rootMutex.unlock();
        return true;
    }

    DevINode::~DevINode() {}

    uint64_t DevINode::lookup(UNUSED const char *name) { return -1; }

}; // namespace fs