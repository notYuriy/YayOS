#ifndef __KVMMNGR_HPP_INCLUDED__
#define __KVMMNGR_HPP_INCLUDED__

#include <memory/physbase.hpp>
#include <memory/vmbase.hpp>
#include <proc/mutex.hpp>
#include <utils.hpp>

namespace memory {
#pragma pack(1)
    struct MemoryArea {
        vaddr_t offset;
        uint64_t size;
        MemoryArea *next;
        MemoryArea *prev;
    };
    struct MemoryAreaPool {
        MemoryArea *first;
        MemoryAreaPool *next;
        MemoryAreaPool *prev;
        uint64_t count;
        MemoryArea areas[127];

        void init() {
            count = 127;
            first = &areas[0];
            for (uint64_t i = 0; i < 126; ++i) {
                areas[i].next = &areas[i + 1];
            }
            areas[126].next = nullptr;
        }

        MemoryArea *allocNode() {
            MemoryArea *result = first;
            first = first->next;
            count--;
            return result;
        }

        void freeNode(MemoryArea *node) {
            node->next = first;
            first = node;
            count++;
        }
    };
    static_assert(sizeof(MemoryAreaPool) == 4096);
#pragma pack(0)

    class KernelVirtualAllocator {
        static bool m_initialized;
        static MemoryAreaPool *m_poolHeads[128];
        static uint64_t m_lastCheckedIndex;
        static MemoryArea *m_kernelAreas;
        static proc::Mutex m_kvmmngrMutex;

        static void cutPool(MemoryAreaPool *pool);
        static void insertPool(MemoryAreaPool *pool);
        static MemoryArea *allocMemoryArea();
        static void freeMemoryArea(MemoryArea *node);
        static void insertBefore(MemoryArea *area, MemoryArea *point);
        static void cutNode(MemoryArea *area);
        static void mergeAdjacent(MemoryArea *area);
        static void freeMapping(MemoryArea *area);
        static void freePools();
        static bool allocNewPool();
        static MemoryArea *findBestFit(uint64_t requestedSize);
        static void freeRange(vaddr_t virtualAddr, uint64_t size);

    public:
        static vaddr_t getMapping(uint64_t size, paddr_t physBase,
                                  uint64_t flags);
        static void unmapAt(vaddr_t virtualAddr, uint64_t size);
        static void init();
        static void trace();
        INLINE static bool isInitialized() { return m_initialized; }
    };
}; // namespace memory

#endif