#include <serial.hpp>
#include <kprintf.hpp>
#include <mminit.hpp>
#include <interrupts.hpp>
#include <pic.hpp>
#include <pictimer.hpp>
#include <rsdp.hpp>
#include <rxsdt.hpp>

extern "C" void kmain(Uint64 mbPointer) {
    IO::Serial::init(IO::SerialPort::COM1);
    memory::init(mbPointer);
    interrupts::IDT::init();
    drivers::PIC::detectPIC();
    drivers::PICTimer timer;
    timer.init(20);
    //timer.enable();
    acpi::RSDPManager::init((multiboot::BootInfoHeader*)(mbPointer + KERNEL_MAPPING_BASE));
    acpi::RootTableManager::init();
    while(1);
}