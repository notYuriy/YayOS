typedef unsigned long long int uint64_t;
extern "C" void YY_ConsoleWrite(const char *data, uint64_t size);

uint64_t strlen(const char *str) {
    uint64_t result = 0;
    while (str[result] != '\0') {
        result++;
    }
    return result;
}

void print(const char *msg) { YY_ConsoleWrite(msg, strlen(msg)); }

extern "C" void YY_Main(int argc, char **argv) {
    print("Hello, world from C++ code\n\r");
}