#include <core/gdt.hpp>
#include <core/interrupts.hpp>
#include <core/log.hpp>
#include <core/tss.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <drivers/serial.hpp>
#include <drivers/timer/pit.hpp>
#include <fs/ramdiskfs.hpp>
#include <fs/vfs.hpp>
#include <mm/kvmmngr.hpp>
#include <mm/mminit.hpp>
#include <mm/usrvmmngr.hpp>
#include <proc/elf.hpp>
#include <proc/mutex.hpp>
#include <proc/proc.hpp>
#include <proc/usermode.hpp>

extern "C" void kmain(uint64_t mbPointer, void (**ctorsStart)(),
                      void (**ctorsEnd)()) {

    executeCtors(ctorsStart, ctorsEnd);
    drivers::Serial::init(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    core::GDT::init();
    core::TSS::init();
    core::IDT::init();
    drivers::IPIC::detectPIC();
    drivers::PIT timer;
    timer.init(200);
    proc::ProcessManager::init(&timer);
    timer.enable();
    fs::RamdiskFsSuperblock initRd;
    fs::VFS::init(&initRd);
    fs::IFile *file = fs::VFS::open("/bin/binaryExample", 0);
    proc::Elf *elf = proc::parseElf(file);
    memory::UserVirtualAllocator *allocator = memory::newUserVirtualAllocator();
    if (elf == nullptr) {
        core::log("Elf is nullptr\n\r");
    }
    elf->load(file, allocator);
    core::log("Elf file successfully loaded\n\r");
    proc::jumpToUserMode(elf->head.entryPoint, 0);
    while (1)
        ;
}
