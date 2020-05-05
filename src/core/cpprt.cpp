#include <utils.hpp>

extern "C" void __cxa_pure_virtual() {
    panic("[CppRuntime] Pure virtual function is called");
}

void executeCtors(void (**ctorsStart)(), void (**ctorsEnd)()) {
    do {
        if (*ctorsStart != nullptr) {
            (*ctorsStart)();
        }
    } while (ctorsStart++ != ctorsEnd);
}