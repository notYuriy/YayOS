#include <drivers/pic/pic.hpp>
#include <drivers/pic/pic8259.hpp>
#include <mm/kheap.hpp>

namespace drivers {
    IPIC *IPIC::m_systemPic;
    bool IPIC::m_picInitialized;

    void IPIC::detectPIC() {
        if (!core::IDT::isInitialized()) {
            panic("[PIC] Dependency \"IDT\" is not satisfied");
        }
        if (!memory::KernelHeap::isInitialized()) {
            panic("[PIC] Dependency \"KernelHeap\" is not satisfied");
        }
        PIC8259 *pic = new PIC8259;
        pic->init();
        m_systemPic = pic;
        if (!m_systemPic->isInstanceInitialized()) {
            panic("[PIC] Failed to initialize 8259 controller\n\r");
        }
        m_picInitialized = true;
    }
} // namespace drivers