#include <cstdio>
#include <cstdlib>
#include "config.h"

#if USING(DEV_BUILD)
#define ASSERT(expr) (void)((!!(expr)) || (::internal::_assert(#expr, __FILE__, __LINE__), 0))
#else
#define ASSERT(expr) ((void)0)
#endif

#define PANIC(msg) ::internal::_panic(msg, __FILE__, __LINE__)

namespace internal {

void _assert(const char* expr, const char* file, int ln);

void _panic(const char* msg, const char* file, int ln);

}  // namespace internal
