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

    constexpr uint64_t YY_VirtualFlagsWritable = 1ULL << 0;
    constexpr uint64_t YY_VirtualFlagsExecutable = 1ULL << 1;

    extern "C" int64_t YY_VirtualAlloc(uint64_t pagesCount, uint64_t flags);

    extern "C" int64_t YY_VirtualFree(uint64_t start, uint64_t pagesCount);

    constexpr uint64_t YY_APIInfoId_PageSize = 1;
    constexpr uint64_t YY_APIInfoId_MaxArgLength = 2;
    constexpr uint64_t YY_APIInfoID_MaxArgCount = 3;
    constexpr uint64_t YY_APIInfoID_ExecMaxPathLength = 4;
    constexpr uint64_t YY_APIInfoID_MaxOpenFilePathLength = 5;
    constexpr uint64_t YY_APIInfoID_MaxFileIOBufSize = 6;

    extern "C" int64_t YY_QueryAPIInfo(uint64_t id);

    extern "C" int64_t YY_CheckProcStatus(uint64_t pid);

    constexpr int64_t YY_MaxArgLength = 4096;
    constexpr int64_t YY_MaxArgCount = 4096;
    constexpr int64_t YY_ExecMaxPathLength = 1024;

    extern "C" int64_t YY_ExecuteBinary(const char *path, uint64_t argc,
                                        const char **argv);

    constexpr int64_t YY_MaxOpenFilePath = 4096;

    extern "C" int64_t YY_OpenFile(const char *path, bool writable);

    constexpr int64_t YY_MaxFileIOBufSize = 65536;

    extern "C" int64_t YY_ReadFile(int64_t fd, char *buf, int64_t size);
}; // namespace proc

#endif