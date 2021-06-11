#include "core_assert.h"

namespace internal {

void _assert(const char* expr, const char* file, int ln) {
    fprintf(stderr, "*** assertion failed ***\nassertion (%s) failed on line %d in file [%s]\n", expr, ln, file);
    fflush(stderr);
    __debugbreak();
}

void _panic(const char* msg, const char* file, int ln) {
    fprintf(stderr, "*** panic ***\n(%s) on line %d in file [%s]\n", msg, ln, file);
    fflush(stderr);
    __debugbreak();
}

}  // namespace internal
