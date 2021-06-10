#include <cstdio>
#include <cstdlib>
#include "config.h"

#if USING(DEV_BUILD)
#define ASSERT(expr) (void)((!!(expr)) || (::internal::_assert(#expr, __FILE__, __LINE__), 0))
#else
#define ASSERT(expr) ((void)0)
#endif

namespace internal {

void _assert(const char* expr, const char* file, int ln);

}  // namespace internal
