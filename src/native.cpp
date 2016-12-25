#include <cstdio>

// This file is for testing "extern" usage only.
// Not used in core compiler code.
extern "C"
void printi(long long val)
{
    printf("%lld\n", val);
}