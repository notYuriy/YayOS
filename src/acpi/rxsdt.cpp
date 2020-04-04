#include <kheap.hpp>
#include <kvmmngr.hpp>
#include <rsdp.hpp>
#include <rxsdt.hpp>

namespace acpi {
    void RootTableManager::init() {
        if (!memory::KernelVirtualAllocator::isInitialized()) {
            panic("[RootTableMangager] Dependency \"KernelVirtualAllocator\" "
                  "is not satisfied\n\r");
        }
        if (!memory::KernelHeap::isInitialized()) {
            panic("[RootTableManager] Dependency \"KernelHeap\" is not "
                  "satisfied");
        }
        if (!RSDPManager::isInitialized()) {
            panic("[RootTableManager] Dependency \"RSDPManager\" is not "
                  "satisfied");
        }
        bool xdst = false;
        memory::PAddr physicalAddr = 0;
        if (RSDPManager::getVersion() == 2 &&
            RSDPManager::getV2()->xsdtAddress != 0) {
            physicalAddr = RSDPManager::getV2()->xsdtAddress;
            xdst = true;
        } else {
            physicalAddr = RSDPManager::getV1()->rsdtAddress;
        }
        memory::PAddr sdtBase, sdtLimit;
        sdtBase = alignDown(physicalAddr, 4096);
        sdtLimit = alignDown(physicalAddr + sizeof(SDT), 4096) + 4096;
        memory::VAddr mapping = memory::KernelVirtualAllocator::getMapping(
            sdtLimit - sdtBase, sdtBase, false);
        kprintf("memory::VAddr mapping = 0x%p;", mapping);
    }
}; // namespace acpi