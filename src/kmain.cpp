#include <inttypes.hpp>
#include <kprintf.hpp>
#include <memoryinfo.hpp>
#include <multiboot.hpp>
#include <physalloc.hpp>
#include <serial.hpp>
#include <tmpphysalloc.hpp>
#include <tmpvalloc.hpp>
#include <vmmap.hpp>
#include <kvmmngr.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    kprintf("===============\n\r");
    memory::BootMemoryInfo::init(mbPointer);
    memory::TempPhysAllocator::init();
    memory::TempVirtualAllocator::init(KERNEL_MAPPING_BASE + 128 MB);
    memory::PhysAllocator::init();
    memory::KernelVirtualAllocator::init();
    
    memory::VAddr addr1 = memory::KernelVirtualAllocator::getMapping(40960, 0, true);
    memory::VAddr addr2 = memory::KernelVirtualAllocator::getMapping(40960, 0, true);
    memory::VAddr addr3 = memory::KernelVirtualAllocator::getMapping(40960, 0, true);

    memory::KernelVirtualAllocator::unmapAt(addr1, 40960);

    memory::VAddr addr4 = memory::KernelVirtualAllocator::getMapping(40960, 0, true);

    kprintf("%p %p %p %p\n\r", addr1, addr2, addr3, addr4);

    kprintf("It did not crash!\n\r");
}