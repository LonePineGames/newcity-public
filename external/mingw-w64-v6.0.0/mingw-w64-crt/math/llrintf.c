/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#include <math.h>
#include <fenv.h>

long long llrintf (float x) 
{
  long long retval = 0ll;
#if defined(_AMD64_) || defined(__x86_64__) || defined(_X86_) || defined(__i386__)
  __asm__ __volatile__ ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");
#else
  int mode = fegetround();
  if (mode == FE_DOWNWARD)
    retval = (long long)floorf(x);
  else if (mode == FE_UPWARD)
    retval = (long long)ceilf(x);
  else if (mode == FE_TOWARDZERO)
    retval = x >= 0 ? (long long)floorf(x) : (long long)ceilf(x);
  else
    retval = x >= 0 ? (long long)floorf(x + 0.5) : (long long)ceilf(x - 0.5);
#endif
  return retval;
}
