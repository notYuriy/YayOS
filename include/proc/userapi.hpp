#ifndef __SYSCALLS_HPP_INCLUDED__
#define __SYSCALLS_HPP_INCLUDED__

#include <utils.hpp>

namespace proc {

    [[noreturn]] void YY_ExitProcess();

    extern "C" int64_t YY_DuplicateProcess();

    constexpr uint64_t YY_ConsoleOperationsSizeLimit = 65536;
    extern "C" int64_t YY_ConsoleWrite(char *location, uint64_t size);

    constexpr uint64_t YY_SystemInfoStringLimit = 64;
#pragma pack(1)
    struct YY_SystemInfo {
        char kernelName[YY_SystemInfoStringLimit + 1];
        char kernelRelease[YY_SystemInfoStringLimit + 1];
        char kernelVersion[YY_SystemInfoStringLimit + 1];
        char machine[YY_SystemInfoStringLimit + 1];
        char processor[YY_SystemInfoStringLimit + 1];
        char hardwarePlatform[YY_SystemInfoStringLimit + 1];
        char operatingSystem[YY_SystemInfoStringLimit + 1];
    };
#pragma pack(0)

    static_assert(sizeof(YY_SystemInfo) == 455);

    extern "C" int64_t YY_GetSystemInfo(YY_SystemInfo *info);

    extern "C" void YY_Yield();
}; // namespace proc

#endif