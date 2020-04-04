#include <serial.hpp>
#include <kprintf.hpp>
#include <mminit.hpp>
#include <interrupts.hpp>
#include <pic.hpp>
#include <pictimer.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::init(mbPointer);
    interrupts::IDT::init();
    drivers::PIC::detectPIC();
    drivers::PICTimer timer;
    timer.init(20);
    timer.enable();
    while(1);
}