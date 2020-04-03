#include <inttypes.hpp>
#include <kheap.hpp>
#include <kprintf.hpp>
#include <kvmmngr.hpp>
#include <memoryinfo.hpp>
#include <multiboot.hpp>
#include <physalloc.hpp>
#include <serial.hpp>
#include <tmpphysalloc.hpp>
#include <tmpvalloc.hpp>
#include <vmmap.hpp>
#include <interrupts.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::BootMemoryInfo::init(mbPointer);
    memory::TempPhysAllocator::init();
    memory::TempVirtualAllocator::init(KERNEL_MAPPING_BASE + 128 MB);
    memory::PhysAllocator::init();
    memory::KernelVirtualAllocator::init();
    memory::KernelHeap::init();
    interrupts::Idt::init();
    asm("int $0x80");
    kprintf("It didn't crash!\n\r");
}