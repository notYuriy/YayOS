#include <core/log.hpp>
#include <fs/vfs.hpp>
#include <mm/kheap.hpp>
#include <proc/spinlock.hpp>
#include <stdatomic.h>

namespace fs {

    bool VFS::initialized;
    IFilesystemInstance* VFS::fsList;
    FileTreeNode* root;

    INLINE void atomicIncrement(Uint64* loc) { __sync_fetch_and_add(loc, 1); }

    INLINE bool atomicDecrement(Uint64* loc) {
        __sync_fetch_and_add(loc, 1) == 1;
    }

    struct PathIterator {
        char* pointer;
        Uint64 pos, len, lastIndex;
        INLINE void init(const char* path) {
            Uint64 length = strlen(path);
            char* copy = new char[length + 1];
            memcpy(copy, path, length);
            copy[length] = '\0';
            for (Uint64 i = 0; i <= length; ++i) {
                if (copy[i] == '/') {
                    copy[i] = '\0';
                }
            }
            pointer = copy;
            pos = 0;
            len = length;
        }
        INLINE char* get() { return pointer + pos; }
        INLINE Uint64 getPos() { return pos; }
        INLINE void setPos(Uint64 val) { pos = val; }
        INLINE bool next() {
            while (pos != len && pointer[pos] != '\0') {
                pos++;
            }
            if (pos == len) {
                return false;
            }
            pos++;
            return true;
        }
        INLINE void free() { delete[] pointer; }
    };

    void VFS::init(IFilesystemInstance* rootFS) {
        fsList = rootFS;
        if (!memory::KernelHeap::isInitialized()) {
            panic("[VFS] Dependency \"KernelHeap\" is not satisfied\n\r");
        }
        root = new FileTreeNode;
        root->parent = nullptr;
        root->usedCount = 1;
        root->node = rootFS->getRoot();
        root->subdirs = nullptr;
        root->fromParent = nullptr;
        initialized = true;
    }

    FileTreeListNode* FileTreeNode::lookup(const char* name) {
        Uint64 hash = strhash((const Uint8*)name);
        FileTreeListNode* current = subdirs;
        while (current != nullptr) {
            if (current->hash == hash && streq(current->name, name)) {
                return current;
            }
        }
        return nullptr;
    }

    void FileTreeNode::increaseRefCount() { atomicIncrement(&usedCount); }

    bool FileTreeNode::decreaseRefCount() {
        if (atomicDecrement(&usedCount)) {
            node->sb->drop(node->number);
            if (fromParent->prev == nullptr) {
                parent->subdirs = fromParent->next;
            } else {
                fromParent->prev->next = fromParent->next;
            }
            if (fromParent->next != nullptr) {
                fromParent->next->prev = fromParent->prev;
            }
            delete fromParent->name;
            delete fromParent;
            delete this;
            return true;
        }
        return false;
    }

} // namespace fs