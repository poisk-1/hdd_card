#pragma once

#ifdef DEBUG
#include <xc.h>
#define ASSERT(x) ((void)((x) || (__builtin_software_breakpoint(), 0)))
#else
#define ASSERT(x) (void)0
#endif
