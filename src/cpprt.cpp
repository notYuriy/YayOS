#include <utils.hpp>

extern "C" void __cxa_pure_virtual() {
    panic("[CppRuntime] Pure virtual function is called");
}