#include <interrupts.hpp>
#include <kprintf.hpp>
#include <mminit.hpp>
#include <pic.hpp>
#include <pic8259.hpp>
#include <pit.hpp>
#include <serial.hpp>
#include <proc.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::init(mbPointer);
    interrupts::IDT::init();
    //This code works fine
    //drivers::PIC8259 pic;
    //pic.init();
    //drivers::PIC* pics = &pic;
    //drivers::PIT timer;
    //timer.init(20);
    //proc::ProcessManager::init(&timer);
    
    //this code doesn't
    drivers::PIC::detectPIC();
    drivers::PIT timer;
    timer.init(20);
    proc::ProcessManager::init(&timer);
    timer.enable();

    kprintf("Fine!\n\r");
    while (1) {}
}