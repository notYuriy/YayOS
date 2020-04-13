#include <core/interrupts.hpp>
#include <core/log.hpp>
#include <mm/mminit.hpp>
#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <drivers/timer/pit.hpp>
#include <proc/proc.hpp>
#include <drivers/serial.hpp>
#include <fs/vfs.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    drivers::Serial::init(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    core::IDT::init();
    drivers::IPIC::detectPIC();
    drivers::PIT timer;
    timer.init(20);
    proc::ProcessManager::init(&timer);
    fs::VFS::init(nullptr);
    timer.enable();
    core::log("YayOS up and running\n\r");
    while(true) {}
}