#include <pic.hpp>
#include <pic8259.hpp>

namespace drivers {
    PIC* PIC::systemPic;
    bool PIC::picInitialized;

    PIC8259 pic8259;

    void PIC::detectPIC() {
        if(!interrupts::IDT::isInitialized()) {
            panic("[PIC] Dependency \"IDT\" is not satisfied");
        }
        pic8259.init();
        if(!pic8259.isInstanceInitialized()) {
            panic("[PIC] Failed to initialize 8259 controller\n\r");
        }
        systemPic = &pic8259;
        picInitialized = true;
    }
}