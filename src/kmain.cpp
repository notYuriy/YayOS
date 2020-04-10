#include <interrupts.hpp>
#include <log.hpp>
#include <mminit.hpp>
#include <pic.hpp>
#include <pic8259.hpp>
#include <pit.hpp>
#include <proc.hpp>
#include <serial.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    drivers::Serial::init(drivers::SerialPort::COM1);
    memory::init(mbPointer);
    core::IDT::init();
    drivers::PIC::detectPIC();
    drivers::PIT timer;
    timer.init(20);
    proc::ProcessManager::init(&timer);
    timer.enable();
    core::log("YayOS up and running\n\r");
    while(true) {}
}