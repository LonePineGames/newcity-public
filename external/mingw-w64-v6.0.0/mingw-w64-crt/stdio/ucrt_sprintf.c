/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */

#undef __MSVCRT_VERSION__
#define __MSVCRT_VERSION__ 0x1400
#include <stdio.h>

int __cdecl sprintf(char * __restrict__ _Dest,const char * __restrict__ _Format,...) __MINGW_ATTRIB_DEPRECATED_SEC_WARN
{
  __builtin_va_list ap;
  int ret;
  __builtin_va_start(ap, _Format);
  ret = __stdio_common_vsprintf(UCRTBASE_PRINTF_STANDARD_SNPRINTF_BEHAVIOUR, _Dest, (size_t)-1, _Format, NULL, ap);
  __builtin_va_end(ap);
  return ret;
}
int __cdecl (*__MINGW_IMP_SYMBOL(sprintf))(char *__restrict__, const char *__restrict__, ...) = sprintf;
