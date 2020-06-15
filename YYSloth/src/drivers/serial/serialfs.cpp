#include <drivers/serial/serialfs.hpp>
#include <proc/intlock.hpp>

namespace fs {

    UARTNode::UARTNode(drivers::SerialPort port) { this->port = port; }

    IFile *UARTNode::open(UNUSED bool writable) { return new UARTFile(this); }

    UARTNode::~UARTNode() {}

    UARTFile::UARTFile(UARTNode *node) { this->node = node; }

    int64_t UARTFile::read(int64_t size, uint8_t *buf) {
        for (int64_t i = 0; i < size; ++i) {
            proc::disableInterrupts();
            if (!drivers::Serial::readyToRecieve(node->port)) {
                proc::enableInterrupts();
                return i;
            }
            uint8_t result = drivers::Serial::recieve(node->port, false);
            proc::enableInterrupts();
            buf[i] = result;
        }
        return size;
    }

    int64_t UARTFile::write(int64_t size, const uint8_t *buf) {
        node->mutex.lock();
        for (int64_t i = 0; i < size; ++i) {
            drivers::Serial::send(node->port, buf[i]);
        }
        node->mutex.unlock();
        return size;
    }

}; // namespace fs