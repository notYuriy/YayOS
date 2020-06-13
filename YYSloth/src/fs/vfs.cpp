#include <fs/vfs.hpp>
#include <memory/kheap.hpp>

namespace fs {

    struct DEntry {
        DEntry *par, *next, *prev, *chld, *mnt;
        proc::Mutex mutex;
        bool isFilesystemRoot, isMountpoint;
        // usedCount =
        // number of threads traversing vfs in this DEntry +
        // number of opened file descriptors here +
        // number of children nodes
        uint64_t usedCount;
        uint64_t nameHash;
        ISuperblock *sb;
        INode *node;
        char *name;

        static DEntry *createNode(const char *name);
        DEntry *createChildNode(const char *name);
        DEntry *softLookup(const char *name);
        DEntry *hardLookup(const char *name);
        DEntry *goToParent();
        DEntry *goToChild(const char *name);
        // the last node remain in observed state (so it is not deleted)
        DEntry *walk(PathIterator *iter, bool resolveLast = true);
        void incrementUsedCount();
        void decrementUsedCount();
        void dispose();
        void cut();
        bool drop();
        void dropRec();
    };

    ISuperblock *VFS::m_rootFs[26];
    DEntry *VFS::m_fsTrees[26];
    proc::Mutex *VFS::m_rootMutex;

    DEntry *DEntry::createNode(const char *name) {
        DEntry *newDEntry = new DEntry;
        if (newDEntry == nullptr) {
            return nullptr;
        }
        newDEntry->name = strdup(name);
        newDEntry->nameHash = strhash(name);
        newDEntry->chld = nullptr;
        newDEntry->usedCount = 0;
        newDEntry->node = nullptr;
        newDEntry->isFilesystemRoot = false;
        newDEntry->isFilesystemRoot = false;
        newDEntry->mnt = nullptr;
        return newDEntry;
    }

    DEntry *DEntry::createChildNode(const char *name) {
        DEntry *newDEntry = createNode(name);
        if (newDEntry == nullptr) {
            return nullptr;
        }
        if (chld != nullptr) {
            newDEntry->next = chld;
            newDEntry->prev = chld->prev;
            chld->prev->next = newDEntry;
            chld->prev = newDEntry;
        } else {
            newDEntry->next = newDEntry;
            newDEntry->prev = newDEntry;
        }
        newDEntry->par = this;
        newDEntry->chld = nullptr;
        newDEntry->sb = sb;
        return newDEntry;
    }

    DEntry *DEntry::softLookup(const char *name) {
        uint64_t hash = strhash(name);
        DEntry *current = chld;
        if (current == nullptr) {
            return nullptr;
        }
        do {
            if (current->nameHash == hash) {
                if (streq(current->name, name)) {
                    return this;
                }
            }
            current = current->next;
        } while (current != chld);
        return nullptr;
    }

    DEntry *DEntry::hardLookup(const char *name) {
        if (node == nullptr) {
            return nullptr;
        }
        uint64_t num = node->lookup(name);
        if (num == 0) {
            return nullptr;
        }
        DEntry *newChld = createChildNode(name);
        if (newChld == nullptr) {
            return nullptr;
        }
        newChld->node = sb->getNode(num);
        incrementUsedCount();
        return newChld;
    }

    void DEntry::incrementUsedCount() { usedCount++; }

    void DEntry::decrementUsedCount() { usedCount--; }

    void DEntry::dispose() {
        delete name;
        sb->dropNode(node->num);
        delete this;
    }

    void DEntry::cut() {
        prev->next = next;
        next->prev = prev;
        next = nullptr;
        prev = nullptr;
    }

    bool DEntry::drop() {
        if (mutex.someoneWaiting()) {
            return false;
        }
        if (usedCount > 0) {
            return false;
        }
        par->mutex.lock();
        if (usedCount > 0) {
            par->mutex.unlock();
            return false;
        }
        if (mutex.someoneWaiting()) {
            par->mutex.unlock();
            return false;
        }
        par->decrementUsedCount();
        cut();
        par->mutex.unlock();
        return true;
    }

    DEntry *DEntry::goToParent() {
        if (par == nullptr) {
            return this;
        }
        mutex.lock();
        decrementUsedCount();
        DEntry *result = par;
        // TODO: atomic increment
        // invariant: parent and this node are alive
        // while dispose is not called
        par->incrementUsedCount();
        if (usedCount > 0) {
            mutex.unlock();
        } else {
            bool result = drop();
            mutex.unlock();
            if (result) {
                dispose();
            }
        }
        return result;
    }

    DEntry *DEntry::goToChild(const char *name) {
        mutex.lock();
        // search in cache
        DEntry *result = softLookup(name);
        if (result == nullptr) {
            // add it to cache
            result = hardLookup(name);
            if (result == nullptr) {
                mutex.unlock();
                return nullptr;
            }
        }
        result->incrementUsedCount();
        decrementUsedCount();
        mutex.unlock();
        return result;
    }

    void DEntry::dropRec() {
        DEntry *current = this;
        while (current != nullptr) {
            current->mutex.lock();
            if (!current->drop()) {
                current->mutex.unlock();
                return;
            }
            current->mutex.unlock();
            DEntry *next = current->par;
            dispose();
            current = next;
        }
    }

    DEntry *DEntry::walk(PathIterator *iter, bool resolveLast) {
        DEntry *current = this;
        // Observing this node. Current is not going to be deleted here
        // because
        // 1) it is fs root so it is always mounted and preserved
        // 2) it is opened folder => usedCount at least one
        // incrementing is only done to ensure goToChild/goToParent validity
        current->incrementUsedCount();
        while (!iter->atEnd(resolveLast)) {
            if (*(iter->get()) == '\0' || streq(iter->get(), ".")) {
                iter->next();
                continue;
            }
            if (streq(iter->get(), "..")) {
                current = current->goToParent();
                iter->next();
                continue;
            }
            DEntry *next = current->goToChild(iter->get());
            if (next == nullptr) {
                current->decrementUsedCount();
                current->dropRec();
                return nullptr;
            }
            current = next;
            iter->next();
        }
        return current;
    }

    INLINE int64_t getIndex(char letter) {
        if (('a' <= letter) && (letter <= 'z')) {
            return letter - 'a';
        }
        if (('A' <= letter) && (letter <= 'Z')) {
            return letter - 'A';
        }
        return -1;
    }

    void VFS::init() {
        for (uint64_t i = 0; i < 26; ++i) {
            m_fsTrees[i] = nullptr;
            m_rootFs[i] = nullptr;
        }
        m_rootMutex = new proc::Mutex;
    }

    bool VFS::mount(char letter, ISuperblock *sb) {
        int64_t index = getIndex(letter);
        if (index == -1) {
            return false;
        }
        m_rootMutex->lock();
        if (m_fsTrees[index] == nullptr) {
            m_rootFs[index] = sb;
            m_rootFs[index]->mount();
            m_fsTrees[index] = DEntry::createNode("");
            m_fsTrees[index]->next = m_fsTrees[index];
            m_fsTrees[index]->prev = m_fsTrees[index];
            m_fsTrees[index]->par = m_fsTrees[index];
            m_fsTrees[index]->isFilesystemRoot = true;
            m_fsTrees[index]->sb = sb;
            m_fsTrees[index]->node = sb->getNode(sb->getRootNum());
            m_fsTrees[index]->incrementUsedCount();
            m_rootMutex->unlock();
            return true;
        }
        m_rootMutex->unlock();
        return false;
    }

    IFile *VFS::open(const char *path, int perm) {
        if (strlen(path, 3) != 3) {
            return nullptr;
        }
        int64_t index = getIndex(path[0]);
        if (index == -1) {
            return nullptr;
        }
        PathIterator iter(path + 2);
        m_rootMutex->lock();
        DEntry *root = m_fsTrees[index];
        m_rootMutex->unlock();
        DEntry *entry = root->walk(&iter);
        if (entry == nullptr) {
            return nullptr;
        }
        entry->incrementUsedCount();
        IFile *result = entry->node->open(perm);
        if (result == nullptr) {
            return nullptr;
        }
        result->entry = entry;
        return result;
    }

    IFile::~IFile() {
        entry->decrementUsedCount();
        entry->dropRec();
    }

    INode::~INode() {}
    ISuperblock::~ISuperblock() {}

}; // namespace fs