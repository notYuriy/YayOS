#include <memory/msecurity.hpp>

namespace memory {
    bool isPWritable(PageTable *page, vind_t index) {
        if (page->entries[index].writable) {
            return true;
        }
        if (page->entries[index].cow && page->entries[index].wasWritable) {
            return true;
        }
        return false;
    }
    bool virtualPageConditionCheck(vaddr_t page, bool isUser, bool isWritable,
                                   bool isCode) {
        vind_t p4Index, p3Index, p2Index, p1Index;
        p4Index = getP4Index(page);
        p3Index = getP3Index(page);
        p2Index = getP2Index(page);
        p1Index = getP1Index(page);
        PageTable *p4Table = (PageTable *)P4_TABLE_VIRTUAL_ADDRESS;
        PageTable *p3Table = p4Table->walkTo(p4Index);
        PageTable *p2Table = p3Table->walkTo(p3Index);
        PageTable *p1Table = p2Table->walkTo(p2Index);
        if (!(p4Table->entries[p4Index].present)) {
            return false;
        } else if (!(p3Table->entries[p3Index].present)) {
            return false;
        } else if (!(p2Table->entries[p2Index].present)) {
            return false;
        } else if (!(p1Table->entries[p1Index].present)) {
            return false;
        } else if (isUser && !(p4Table->entries[p4Index].userAccessible)) {
            return false;
        } else if (isUser && !(p3Table->entries[p3Index].userAccessible)) {
            return false;
        } else if (isUser && !(p2Table->entries[p2Index].userAccessible)) {
            return false;
        } else if (isUser && !(p1Table->entries[p1Index].userAccessible)) {
            return false;
        } else if (isCode &&
                   (p4Table->entries[p4Index].addr & ((1ULL << 63) != 0))) {
            return false;
        } else if (isCode &&
                   (p3Table->entries[p3Index].addr & ((1ULL << 63) != 0))) {
            return false;
        } else if (isCode &&
                   (p2Table->entries[p2Index].addr & ((1ULL << 63) != 0))) {
            return false;
        } else if (isCode &&
                   (p1Table->entries[p1Index].addr & ((1ULL << 63) != 0))) {
            return false;
        } else if (isWritable && !(isPWritable(p4Table, p4Index))) {
            return false;
        } else if (isWritable && !(isPWritable(p3Table, p3Index))) {
            return false;
        } else if (isWritable && !(isPWritable(p2Table, p2Index))) {
            return false;
        } else if (isWritable && !(isPWritable(p1Table, p1Index))) {
            return false;
        }
        return true;
    }

    bool virtualRangeConditionCheck(vaddr_t start, uint64_t size, bool isUser,
                                    bool isWritable, bool isCode) {
        vaddr_t begin = alignDown(start, 0x1000);
        vaddr_t end = alignUp(start + size, 0x1000);
        for (vaddr_t page = begin; page < end; page += 0x1000) {
            if (!virtualPageConditionCheck(page, isUser, isWritable, isCode)) {
                return false;
            }
        }
        return true;
    }

    bool validateCString(const char *start, bool isUser, bool isWritable,
                         bool isCode, uint64_t maxLength) {
        for (uint64_t i = 0; i <= maxLength; ++i) {
            if (i % 4096 == 0) {
                if (!virtualPageConditionCheck(
                        alignDown(((vaddr_t)start) + i, 4096), isUser,
                        isWritable, isCode)) {
                    return false;
                }
            }
            if (start[i] == '\0') {
                return true;
            }
        }
        return false;
    }
}; // namespace memory