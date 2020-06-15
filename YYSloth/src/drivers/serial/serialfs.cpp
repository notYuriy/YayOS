#include <drivers/serial/serialfs.hpp>

namespace fs {

    UARTNode::UARTNode(drivers::SerialPort port) { this->port = port; }

    IFile *UARTNode::open(UNUSED bool writable) { return new UARTFile(this); }

    UARTNode::~UARTNode() {}

    UARTFile::UARTFile(UARTNode *node) { this->node = node; }

    // TODO: implement reading with circular buffers and ints
    int64_t UARTFile::read(UNUSED int64_t size, UNUSED uint8_t *buf) {
        return -1;
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