#include "core_assert.h"

#if defined(_WIN32)
#define DEBUGBREAK __debugbreak()
#elif defined(__APPLE__)
#define DEBUGBREAK __builtin_trap()
#else
#error Unknown platform
#endif

namespace internal {

void _assert(const char* expr, const char* file, int ln) {
    fprintf(stderr, "*** assertion failed ***\nassertion (%s) failed on line %d in file [%s]\n", expr, ln, file);
    fflush(stderr);
    DEBUGBREAK;
}

void _panic(const char* msg, const char* file, int ln) {
    fprintf(stderr, "*** panic ***\n(%s) on line %d in file [%s]\n", msg, ln, file);
    fflush(stderr);
    DEBUGBREAK;
}

}  // namespace internal
