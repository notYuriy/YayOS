#ifndef __DESCRIPTOR_HPP_INCLUDED__
#define __DESCRIPTOR_HPP_INCLUDED__

#include <core/dynarray.hpp>
#include <fs/usertypes.hpp>
#include <utils.hpp>

namespace proc {
    struct IDescriptor {
        virtual int64_t read(int64_t size, uint8_t *buf);
        virtual int64_t write(int64_t size, const uint8_t *buf);
        virtual int64_t readdir(int64_t count, fs::Dirent *buf);
        virtual int64_t lseek(int64_t offset, int64_t whence);
        virtual int64_t ltellg();
        virtual int64_t handleCmd(int64_t cmd, void *buf);
        virtual void flush();
        virtual ~IDescriptor();
    };

    struct DescriptorHandle {
        IDescriptor *val;
        struct Mutex *mutex;
        uint64_t refCount;
        DescriptorHandle(IDescriptor *desc);
        DescriptorHandle *clone();
        void release();
    };

    struct LocalDescriptorHandle {
        DescriptorHandle *handle;
        bool closeOnExec;
    };

    class DescriptorTable {
        core::DynArray<LocalDescriptorHandle> *handles;
        void shrinkToFit();

    public:
        bool reinit();
        void clear();
        DescriptorHandle *getDescriptor(int64_t fd);
        bool setDescriptor(int64_t fd, DescriptorHandle *desc);
        bool updateCloseOnExec(int64_t fd, bool closeOnExec);
        int64_t allocDescriptor();
        int64_t freeDescriptor(int64_t fd);
        bool copy(DescriptorTable *dest);
        void onExec();
    };
}; // namespace proc

#endif