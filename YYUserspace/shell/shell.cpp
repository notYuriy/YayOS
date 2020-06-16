typedef unsigned long long int uint64_t;
typedef long long int int64_t;
typedef unsigned short uint16_t;

// Length limit for YY_SystemInfo strings
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

constexpr int64_t YY_Running = 1;
constexpr int64_t YY_ExitedAfterSyscall = 2;
constexpr int64_t YY_NonRecoverableError = 3;

#pragma pack(1)
struct YY_ProcessStatus {
    int64_t returnCode;
    int64_t status;
};
#pragma pack(0)

extern "C" int64_t YY_OpenFile(const char *path, bool writable);
extern "C" int64_t YY_ReadFile(int64_t fd, char *buf, uint64_t count);
extern "C" int64_t YY_WriteFile(int64_t fd, const char *buf, uint64_t count);
extern "C" int64_t YY_DuplicateProcess();
extern "C" int64_t YY_ExecuteBinary(const char *name, uint64_t argc,
                                    char **argv);
extern "C" int64_t YY_GetProcessStatus(int64_t pid, YY_ProcessStatus *stat);
extern "C" int64_t YY_GetSystemInfo(YY_SystemInfo *buf);
extern "C" void YY_ConsoleWrite(const char *msg, uint64_t size);
extern "C" void YY_Yield();
extern "C" void YY_ExitProcess();
extern "C" void YY_CloseFile(int64_t fd);
extern "C" void run(const char *name, uint64_t argc, char **argv,
                    YY_ProcessStatus *stat);

constexpr uint64_t YY_FileNameMaxLength = 255;

struct YY_Dirent {
    uint64_t inodeNumber;
    uint16_t nameLength;
    char name[YY_FileNameMaxLength + 1];
};

extern "C" int64_t YY_ReadDirectory(int64_t fd, YY_Dirent *buf, int64_t size);

uint64_t strlen(const char *str) {
    uint64_t result = 0;
    while (str[result] != '\0') {
        result++;
    }
    return result;
}

int64_t serialFd;

void putc(char c) {
    if (c == '\0') {
        return;
    }
    if (c == '\n') {
        YY_WriteFile(serialFd, "\n\r", 2);
        return;
    }
    YY_WriteFile(serialFd, &c, 1);
}

void puts(const char *msg) {
    uint64_t len = strlen(msg);
    for (uint64_t i = 0; i < len; ++i) {
        putc(msg[i]);
    }
}

void memset(char *mem, uint64_t count, char c) {
    for (uint64_t i = 0; i < count; ++i) {
        mem[i] = c;
    }
}

void memcpy(char *dst, const char *src, uint64_t count) {
    for (uint64_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

char getc() {
    static char buf[1024];
    static int64_t index = 0;
    static int64_t read = 0;
    if (index < read) {
        return buf[index++];
    }
    while ((read = YY_ReadFile(serialFd, buf, 1024)) == 0) {
        asm("pause");
    }
    index = 0;
    return buf[index++];
}

void insertChar(char *buf, uint64_t pos, uint64_t len, char c) {
    char ex = c;
    for (uint64_t i = pos; i <= len; ++i) {
        char tmp = buf[i];
        buf[i] = ex;
        ex = tmp;
    }
}

void removeChar(char *buf, uint64_t pos, uint64_t len) {
    for (uint64_t i = pos; i < len - 1; ++i) {
        buf[i] = buf[i + 1];
    }
}

void render(const char *buf, uint64_t count, uint64_t prevCount, uint64_t pos) {
    puts("\033[0m \b");
    for (uint64_t i = 0; i < prevCount; ++i) {
        puts("\b \b");
    }
    for (uint64_t i = 0; i < count; ++i) {
        if (pos == i) {
            puts("\033[47m\033[30m");
        }
        putc(buf[i]);
        if (pos == i) {
            puts("\033[0m");
        }
    }
    if (pos == count) {
        puts("\033[47m\033[30m \033[0m\b");
    }
}

bool streq(const char *s1, const char *s2) {
    while (*s1 != '\0') {
        if (*s1++ != *s2++) {
            return false;
        }
    }
    if (*s2 != '\0') {
        return false;
    }
    return true;
}

uint64_t getline(char *buf, uint64_t size) {
    uint64_t pos = 0;
    uint64_t length = 0;
    uint64_t prevLength = 0;
    while (true) {
        // view
        render(buf, length, prevLength, pos);
        prevLength = length;
        // controller
        char c = getc();
        // model
        if (c == '\r') {
            break;
        } else if (c == '\b' || c == '\x7f') {
            if (pos != 0) {
                removeChar(buf, pos, length--);
                --pos;
            }
        } else if (c == '\033') {
            char c = getc();
            if (c != '[') {
                putc(c);
                if (length != size) {
                    insertChar(buf, pos++, length++, c);
                }
            }
            c = getc();
            if (c != 'A' && c != 'B' && c != 'C' && c != 'D') {
                if (length != size) {
                    insertChar(buf, pos++, length++, '[');
                }
                if (length != size) {
                    insertChar(buf, pos++, length++, c);
                }
            }
            if (c == 'A' || c == 'B') {
                continue;
            }
            if (c == 'D') {
                if (pos != 0) {
                    // puts("\033[D");
                    pos--;
                }
            } else {
                if (pos != length) {
                    // puts("\033[1C");
                    pos++;
                }
            }
        } else {
            if (length != size) {
                insertChar(buf, pos++, length++, c);
            }
        }
    }
    render(buf, length, prevLength, (uint64_t)(-1));
    puts("\n");
    return length;
}

void shell() {
    char buf[128];
    memset(buf, 128, '\0');
    bool success = true;
    while (true) {
        puts("$ ");
        uint64_t count = getline(buf, 127);
        uint64_t argc = 0;
        char *argv[128];
        for (uint64_t i = 0; i < count; ++i) {
            if (buf[i] != ' ') {
                bool scoped = (buf[i] == '\'' || buf[i] == '\"');
                char start = buf[i];
                argv[argc++] = buf + (i + (scoped ? 1 : 0));
                ++i;
                bool found = false;
                while (i < count) {
                    if (scoped) {
                        if (buf[i] == start) {
                            found = true;
                            break;
                        }
                    } else {
                        if (buf[i] == ' ') {
                            break;
                        }
                    }
                    ++i;
                }
                if ((!found) && scoped) {
                    argc--;
                }
                buf[i] = '\0';
            }
        }
        if (argc == 0) {
            continue;
        }
        if (streq(argv[0], "ls")) {
            if (argc != 2) {
                puts("Command \'ls\' requires one argument\n");
                continue;
            }
            int64_t fd = YY_OpenFile(argv[1], false);
            if (fd == -1) {
                puts("Can't open file \'");
                puts(argv[1]);
                puts("\'\n");
                continue;
            }
            YY_Dirent dirent;
            bool newlinePrinted = false;
            while (true) {
                int64_t read = YY_ReadDirectory(fd, &dirent, 1);
                if (read == -1) {
                    puts("Can't read directory \'");
                    puts(argv[1]);
                    puts("\'\n");
                    break;
                }
                if (read == 0) {
                    if (!newlinePrinted) {
                        puts("\n");
                    }
                    break;
                }
                puts(dirent.name);
                puts("\n");
                newlinePrinted = true;
            }
            YY_CloseFile(fd);
        } else if (streq(argv[0], "echo")) {
            for (uint64_t i = 1; i < argc; ++i) {
                puts(argv[i]);
                puts(" ");
            }
            if (argc != 1) {
                puts("\n");
            }
        } else if (streq(argv[0], "cat")) {
            if (argc != 2) {
                puts("\'cat\' requires one argument\n");
                continue;
            }
            int64_t fd = YY_OpenFile(argv[1], false);
            if (fd == -1) {
                puts("Unable to open file");
                puts(argv[1]);
                puts("\n");
                continue;
            }
            char filebuf[1024];
            memset(filebuf, 1024, '\0');
            int64_t read;
            while ((read = YY_ReadFile(fd, filebuf, 1024)) > 0) {
                for (int64_t i = 0; i < read; ++i) {
                    putc(filebuf[i]);
                }
            }
            if (read == -1) {
                puts("Can't read from \'");
                puts(argv[1]);
                puts("\' file");
            }
            puts("\n");
        } else if ((streq(argv[0], "help"))) {
            puts("Builtins:\n");
            puts("help: print this message\n");
            puts("cat: read file contents\n");
            puts("ls: output directory contents\n");
            puts("echo: print all arguments back\n");
            puts("exit: exit shell\n");
            puts("clear: clear screen\n");
            puts("?: check if last program terminated succesfully\n");
            puts("everything else: run file \'everything else\' as an "
                 "executable\n");
        } else if (streq(argv[0], "exit")) {
            if (argc != 1) {
                puts("\'exit\' command does not take any arguments\n");
                continue;
            }
            YY_ExitProcess();
        } else if (streq(argv[0], "?")) {
            if (argc != 1) {
                puts("\'?\' command does not take any arguments\n");
                continue;
            }
            if (success) {
                puts("success\n");
            } else {
                puts("error\n");
            }
        } else if (streq(argv[0], "clear")) {
            puts("\033[2J\033[H\e[?25l");
        } else {
            YY_ProcessStatus buf;
            run(argv[0], argc, argv, &buf);
            if (buf.status == YY_NonRecoverableError) {
                puts("Non recoverable error happened while executing \'");
                puts(argv[0]);
                puts("\'\n");
            } else if (buf.returnCode == 0xcafebabedeadbeef) {
                puts("Failed to run \'");
                puts(argv[0]);
                puts("\' as an executable\n");
            } else {
                success = true;
                if (buf.status != YY_ExitedAfterSyscall) {
                    success = false;
                }
                if (buf.returnCode != 0) {
                    success = false;
                }
            }
        }
        memset(buf, 128, '\0');
    }
}

extern "C" void YY_Main(int argc, char **argv) {
    serialFd = YY_OpenFile("D:\\COM1", true);
    puts("\033[2J\033[H\e[?25l");
    shell();
}