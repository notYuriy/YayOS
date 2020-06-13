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
} // namespace proc