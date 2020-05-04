#include <core/interrupts.hpp>
#include <core/log.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <drivers/serial.hpp>
#include <drivers/timer/pit.hpp>
#include <fs/ramdiskfs.hpp>
#include <fs/vfs.hpp>
#include <mm/kvmmngr.hpp>
#include <mm/mminit.hpp>
#include <proc/mutex.hpp>
#include <proc/sched.hpp>

extern "C" void kmain(uint64_t mbPointer) {
    drivers::Serial::init(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    core::IDT::init();
    drivers::IPIC::detectPIC();
    drivers::PIT timer;
    timer.init(200);
    proc::Scheduler::init(&timer);
    timer.enable();
    fs::RamdiskFsSuperblock initRd;
    fs::VFS::init(&initRd);
    fs::IFile *file = fs::VFS::open("/A/C/test.txt", 0);
    file->lseek(3, fs::SEEK_SET);
    core::log("file pos: %llu\n\r", file->ltellg());
    uint8_t buf[100];
    memset(buf, 100, '\0');
    file->read(99, buf);
    core::log("buf: %s\n\r", buf);
    fs::VFS::close(file);
    while (1) {
    }
}
