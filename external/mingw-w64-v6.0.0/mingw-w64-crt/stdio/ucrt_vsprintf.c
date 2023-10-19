/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */

#undef __MSVCRT_VERSION__
#define __MSVCRT_VERSION__ 0x1400
#include <stdio.h>

int __cdecl vsprintf(char * __restrict__ _Dest,const char * __restrict__ _Format,va_list _Args) __MINGW_ATTRIB_DEPRECATED_SEC_WARN
{
  return __stdio_common_vsprintf(UCRTBASE_PRINTF_STANDARD_SNPRINTF_BEHAVIOUR, _Dest, (size_t)-1, _Format, NULL, _Args);
}
int __cdecl (*__MINGW_IMP_SYMBOL(vsprintf))(char *__restrict__, const char *__restrict__, va_list) = vsprintf;
