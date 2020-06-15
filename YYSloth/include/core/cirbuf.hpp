#ifndef __CIRCULAR_BUFFER_HPP_INCLUDED__
#define __CIRCULAR_BUFFER_HPP_INCLUDED__

#include <proc/intlock.hpp>
#include <utils.hpp>

namespace core {

    template <class T> struct CircularBuffer {
        uint64_t pos, size;
        T *buf;

        struct CircularBufferView<T> *getNewView() {
            CircularBufferView *view = new CircularBufferView;
            if (view == nullptr) {
                return nullptr;
            }
            proc::disableInterrupts();
            view->pos = pos;
            view->size = size;
            view->buf = this;
            proc::enableInterrupts();
            return view;
        }

        void
        append(T elem) {
            buf[(pos++) % size] = elem;
        }
    };

    template <class T> struct CircularBufferView {
        uint64_t pos, size;
        CircularBuffer *buffer;
        bool read(T *buf) {
            proc::disableInterrupts();
            // nothing to read
            if (pos == buffer->pos) {
                proc::enableInterrupts();
                return false;
            }
            // handling discarded events
            if (pos + size < buffer->pos) {
                pos = (buffer->pos - size);
            }
            *buf = buffer->buf[(pos++) % size];
            proc::enableInterrupts();
            return true;
        }
    };

} // namespace core

#endif