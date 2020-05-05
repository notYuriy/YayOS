#include <mm/usrvmmngr.hpp>

namespace memory {
    UserVirtualMemoryArea::UserVirtualMemoryArea(memory::vaddr_t st,
                                                 uint64_t s) {
        start = st;
        size = s;
        prev = next = nullptr;
    }

    UserVirtualAllocator *newUserVirtualAllocator() {
        UserVirtualAllocator *node = new UserVirtualAllocator;
        if (node == nullptr) {
            return nullptr;
        }
        node->m_head = new UserVirtualMemoryArea(0x1000, 0x1000000000000);
        if (node->m_head == nullptr) {
            return nullptr;
        }
        return node;
    }

    UserVirtualMemoryArea *UserVirtualAllocator::findBestFit(uint64_t size) {
        UserVirtualMemoryArea *current = m_head;
        UserVirtualMemoryArea *result = nullptr;
        while (current != nullptr) {
            if (current->size > size) {
                if (result == nullptr || result->size > current->size) {
                    result = current;
                }
            }
            current = current->next;
        }
        return result;
    }

    void UserVirtualAllocator::cut(UserVirtualMemoryArea *area) {
        if (area->next != nullptr) {
            area->next->prev = area->prev;
        }
        if (area->prev == nullptr) {
            m_head = area->next;
        } else {
            area->prev->next = area->next;
        }
    }

    memory::vaddr_t UserVirtualAllocator::alloc(uint64_t size) {
        UserVirtualMemoryArea *area = findBestFit(size);
        if (area == nullptr) {
            return 0;
        }
        memory::vaddr_t offset = area->start;
        area->start += size;
        area->size -= size;
        if (area->size == 0) {
            cut(area);
            delete area;
        }
        return offset;
    }

    bool UserVirtualAllocator::cutFrom(UserVirtualMemoryArea *area,
                                       memory::vaddr_t start, uint64_t size) {
        if (start == area->start) {
            area->start += size;
            area->size -= size;
            return true;
        }
        if (start + size == area->start + area->size) {
            area->size = start - area->start;
            return true;
        }
        UserVirtualMemoryArea *area2 =
            new UserVirtualMemoryArea(area->start, size - area->start);
        if (area2 == nullptr) {
            return false;
        }
        area->size = area->start + area->size - (start + size);
        area->start = start + size;
        return true;
    }

    bool UserVirtualMemoryArea::in(memory::vaddr_t addr) {
        return (start <= addr) && (addr < (start + size));
    }

    bool UserVirtualAllocator::reserve(memory::vaddr_t addr, uint64_t size) {
        UserVirtualMemoryArea *current = m_head;
        while (current != nullptr) {
            if (current->in(addr)) {
                if (current->in(addr + size)) {
                    return cutFrom(current, addr, size);
                }
                return false;
            }
            current = current->next;
        }
        return false;
    }

    UserVirtualMemoryArea *
    UserVirtualAllocator::findLastBefore(memory::vaddr_t addr) {
        UserVirtualMemoryArea *current = m_head, *result = nullptr;
        while (current != nullptr) {
            if (addr < (current->start + current->size)) {
                return result;
            }
            result = current;
            current = current->next;
        }
        return result;
    }

    bool UserVirtualAllocator::free(memory::vaddr_t addr, uint64_t size) {
        UserVirtualMemoryArea *current = findLastBefore(addr);
        if (LIKELY(current != nullptr && current->next != nullptr &&
                   addr + size < current->next->start)) {
            UserVirtualMemoryArea *newArea =
                new UserVirtualMemoryArea(addr, size);
            if (newArea == nullptr) {
                return false;
            }
            newArea->next = current->next;
            newArea->prev = current;
            current->next = newArea;
            if (newArea->next != nullptr)
                newArea->next->prev = newArea;
            return true;
        }
        if (current != nullptr && current->next == nullptr) {
            UserVirtualMemoryArea *newArea =
                new UserVirtualMemoryArea(addr, size);
            if (newArea == nullptr) {
                return false;
            }
            newArea->prev = current;
            current->next = newArea;
            return true;
        }
        if (UNLIKELY(current == nullptr && m_head == nullptr)) {
            UserVirtualMemoryArea *newArea =
                new UserVirtualMemoryArea(addr, size);
            if (newArea == nullptr) {
                return false;
            }
            m_head = newArea;
            return true;
        }
        if (UNLIKELY(current == nullptr && addr + size < m_head->start)) {
            UserVirtualMemoryArea *newArea =
                new UserVirtualMemoryArea(addr, size);
            if (newArea == nullptr) {
                return false;
            }
            newArea->next = m_head;
            m_head->prev = newArea;
            m_head = newArea;
            return true;
        }
        UserVirtualMemoryArea *left = m_head;
        if (current != nullptr) {
            left = current->next;
        }
        UserVirtualMemoryArea *right = left;
        while (right->next != nullptr && right->start <= (addr + size)) {
            right = right->next;
        }
        memory::vaddr_t newStart = addr;
        uint64_t newSize = size;
        if (newStart > left->start) {
            newStart = left->start;
        }
        if (right->start + right->size > (addr + size)) {
            newSize = (right->start + right->size) - newStart;
        } else {
            newSize = (addr + size) - newStart;
        }
        UserVirtualMemoryArea *cur = left->next;
        UserVirtualMemoryArea *after = right->next;
        while (cur != nullptr && cur->start <= right->start) {
            UserVirtualMemoryArea *next = cur->next;
            delete cur;
            cur = next;
        }
        if (after != nullptr) {
            after->prev = left;
        }
        left->next = after;
        left->start = newStart;
        left->size = newSize;
        return true;
    }
}; // namespace memory