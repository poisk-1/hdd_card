#pragma once

#ifdef DEBUG
#include <stdio.h>
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) do {} while(0)
#endif
