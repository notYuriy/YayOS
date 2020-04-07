#include <interrupts.hpp>
#include <kprintf.hpp>
#include <mminit.hpp>
#include <pic.hpp>
#include <pit.hpp>
#include <serial.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::init(mbPointer);
    memory::KernelHeap::alloc(1000);
    kprintf("Ok\n\r");
    //drivers::PIC::detectPIC();
    //drivers::PIT timer;
    //timer.init(20);
    //timer.enable();
    while (1) {}
}