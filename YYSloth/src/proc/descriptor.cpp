#include <proc/descriptor.hpp>
#include <proc/mutex.hpp>

namespace proc {

    IDescriptor::~IDescriptor() {}

    DescriptorHandle *DescriptorHandle::clone() {
        mutex->lock();
        ++refCount;
        mutex->unlock();
        return this;
    }

    void DescriptorHandle::release() {
        mutex->lock();
        --refCount;
        if (refCount == 0) {
            delete mutex;
            delete val;
            delete this;
        } else {
            mutex->unlock();
        }
    }

    DescriptorHandle::DescriptorHandle(IDescriptor *desc) {
        mutex = new Mutex;
        val = desc;
        refCount = 1;
    }

    int64_t IDescriptor::read(UNUSED int64_t size, UNUSED uint8_t *buf) {
        return -1;
    }

    int64_t IDescriptor::write(UNUSED int64_t size, UNUSED const uint8_t *buf) {
        return -1;
    }

    int64_t IDescriptor::readdir(UNUSED int64_t count, UNUSED fs::Dirent *buf) {
        return -1;
    }

    int64_t IDescriptor::lseek(UNUSED int64_t offset, UNUSED int64_t whence) {
        return -1;
    }

    int64_t IDescriptor::handleCmd(UNUSED int64_t request,
                                   UNUSED void *pointer) {
        return -1;
    }

    int64_t IDescriptor::ltellg() { return -1; }

    void IDescriptor::flush() {}

    void DescriptorTable::shrinkToFit() {
        uint64_t size = handles->size();
        for (uint64_t ind = size; ind > 0; --ind) {
            if (handles->at(ind - 1).handle != nullptr) {
                break;
            }
            size--;
        }
        handles->resize(size);
    }

    bool DescriptorTable::reinit() {
        handles = new core::DynArray<LocalDescriptorHandle>;
        if (handles == nullptr) {
            return false;
        }
        return true;
    }

    void DescriptorTable::clear() {
        delete handles;
        handles = nullptr;
    }

    DescriptorHandle *DescriptorTable::getDescriptor(int64_t fd) {
        if (fd < 0 || fd >= (int64_t)(handles->size())) {
            return nullptr;
        }
        return handles->at(fd).handle;
    }

    bool DescriptorTable::setDescriptor(int64_t fd, DescriptorHandle *desc) {
        if (fd < 0 || fd >= (int64_t)(handles->size())) {
            return false;
        }
        handles->at(fd).handle = desc;
        return true;
    }

    bool DescriptorTable::updateCloseOnExec(int64_t fd, bool closeOnExec) {
        if (fd < 0 || fd >= (int64_t)(handles->size())) {
            return false;
        }
        handles->at(fd).closeOnExec = closeOnExec;
        return true;
    }

    int64_t DescriptorTable::allocDescriptor() {
        for (uint64_t i = 0; i < handles->size(); ++i) {
            if (handles->at(i).handle == nullptr) {
                handles->at(i).closeOnExec = false;
                return i;
            }
        }
        if (!handles->pushBack(LocalDescriptorHandle())) {
            return -1;
        }
        handles->at(handles->size() - 1).closeOnExec = false;
        return handles->size() - 1;
    }

    int64_t DescriptorTable::freeDescriptor(int64_t fd) {
        if (fd < 0 || fd >= (int64_t)(handles->size())) {
            return false;
        }
        handles->at(fd).handle = nullptr;
        shrinkToFit();
        return true;
    }

    bool DescriptorTable::copy(DescriptorTable *dest) {
        if (dest->handles == nullptr) {
            if (!dest->reinit()) {
                return false;
            }
        }
        for (uint64_t i = 0; i < handles->size(); ++i) {
            if (!dest->handles->pushBack(handles->at(i))) {
                dest->handles->clear();
                return false;
            }
        }
        return true;
    }

    void DescriptorTable::onExec() {
        for (uint64_t i = 0; i < handles->size(); ++i) {
            if (handles->at(i).handle == nullptr) {
                continue;
            }
            if (handles->at(i).closeOnExec) {
                handles->at(i).handle = nullptr;
            }
        }
        shrinkToFit();
    }

} // namespace proc