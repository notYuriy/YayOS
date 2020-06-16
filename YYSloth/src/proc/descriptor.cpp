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

} // namespace proc