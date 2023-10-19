#define MINGW_HAS_SECURE_API 1
#include <windows.h>
#include <malloc.h>
#include <errno.h>
#include <msvcrt.h>
#include <sec_api/conio_s.h>

int __cdecl (*__MINGW_IMP_SYMBOL(_cprintf_s_l))(const char *, _locale_t, ...) = 
 _cprintf_s_l;

int __cdecl
_cprintf_s_l (const char *s, _locale_t loc, ...)
{
  va_list argp;
  int r;

  va_start (argp, loc);
  r = _vcprintf_s_l (s, loc, argp);
  va_end (argp);
  return r; 
}
