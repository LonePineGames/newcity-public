/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
#endif

#undef __MSVCRT_VERSION__
#define __MSVCRT_VERSION__ 0x1400

#define fwprintf real_fwprintf
#define _snwprintf real__snwprintf
#define __getmainargs crtimp___getmainargs
#define __wgetmainargs crtimp___wgetmainargs
#define _amsg_exit crtimp__amsg_exit
#define _get_output_format crtimp__get_output_format

#include <internal.h>
#include <sect_attribs.h>
#include <stdio.h>
#include <time.h>

#undef fwprintf
#undef _snwprintf
#undef __getmainargs
#undef __wgetmainargs
#undef _amsg_exit
#undef _get_output_format



// Declarations of non-static functions implemented within this file (that aren't
// declared in any of the included headers, and that isn't mapped away with a define
// to get rid of the _CRTIMP in headers).
int __cdecl __getmainargs(int * _Argc, char *** _Argv, char ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
int __cdecl __wgetmainargs(int * _Argc, wchar_t *** _Argv, wchar_t ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
void __cdecl _amsg_exit(int ret);
unsigned int __cdecl _get_output_format(void);

int __cdecl fwprintf(FILE *ptr, const wchar_t *fmt, ...);
int __cdecl _snwprintf(wchar_t * restrict _Dest, size_t _Count, const wchar_t * restrict _Format, ...);

int __cdecl __ms_fwprintf(FILE *, const wchar_t *, ...);

// Declarations of functions from ucrtbase.dll that we use below
_CRTIMP int* __cdecl __p___argc(void);
_CRTIMP char*** __cdecl __p___argv(void);
_CRTIMP wchar_t*** __cdecl __p___wargv(void);
_CRTIMP char*** __cdecl __p__environ(void);
_CRTIMP wchar_t*** __cdecl __p__wenviron(void);

_CRTIMP int __cdecl _crt_atexit(_onexit_t func);

_CRTIMP int __cdecl _initialize_narrow_environment(void);
_CRTIMP int __cdecl _initialize_wide_environment(void);
_CRTIMP int __cdecl _configure_narrow_argv(int mode);
_CRTIMP int __cdecl _configure_wide_argv(int mode);



// Wrappers with legacy msvcrt.dll style API, based on the new ucrtbase.dll functions.
int __cdecl __getmainargs(int * _Argc, char *** _Argv, char ***_Env, int _DoWildCard, _startupinfo *_StartInfo)
{
  _initialize_narrow_environment();
  _configure_narrow_argv(_DoWildCard ? 2 : 1);
  *_Argc = *__p___argc();
  *_Argv = *__p___argv();
  *_Env = *__p__environ();
  __set_app_type(_StartInfo->newmode);
  return 0;
}

int __cdecl __wgetmainargs(int * _Argc, wchar_t *** _Argv, wchar_t ***_Env, int _DoWildCard, _startupinfo *_StartInfo)
{
  _initialize_wide_environment();
  _configure_wide_argv(_DoWildCard ? 2 : 1);
  *_Argc = *__p___argc();
  *_Argv = *__p___wargv();
  *_Env = *__p__wenviron();
  __set_app_type(_StartInfo->newmode);
  return 0;
}

_onexit_t __cdecl _onexit(_onexit_t func)
{
  return _crt_atexit(func) == 0 ? func : NULL;
}

_onexit_t __cdecl (*__MINGW_IMP_SYMBOL(_onexit))(_onexit_t func) = _onexit;

void __cdecl _amsg_exit(int ret) {
  fprintf(stderr, "runtime error %d\n", ret);
}

unsigned int __cdecl _get_output_format(void)
{
  return 0;
}

static char ** local__initenv;
static wchar_t ** local__winitenv;
char *** __MINGW_IMP_SYMBOL(__initenv) = &local__initenv;
wchar_t *** __MINGW_IMP_SYMBOL(__winitenv) = &local__winitenv;


// These are required to provide the unrepfixed data symbols "timezone"
// and "tzname"; we can't remap "timezone" via a define due to clashes
// with e.g. "struct timezone".
typedef void __cdecl (*_tzset_func)(void);
extern _tzset_func __MINGW_IMP_SYMBOL(_tzset);

// Default initial values until _tzset has been called; these are the same
// as the initial values in msvcrt/ucrtbase.
static char initial_tzname0[] = "PST";
static char initial_tzname1[] = "PDT";
static char *initial_tznames[] = { initial_tzname0, initial_tzname1 };
static long initial_timezone = 28800;
static int initial_daylight = 1;
char** __MINGW_IMP_SYMBOL(tzname) = initial_tznames;
long * __MINGW_IMP_SYMBOL(timezone) = &initial_timezone;
int * __MINGW_IMP_SYMBOL(daylight) = &initial_daylight;

void __cdecl _tzset(void)
{
  __MINGW_IMP_SYMBOL(_tzset)();
  // Redirect the __imp_ pointers to the actual data provided by the UCRT.
  // From this point, the exposed values should stay in sync.
  __MINGW_IMP_SYMBOL(tzname) = _tzname;
  __MINGW_IMP_SYMBOL(timezone) = __timezone();
  __MINGW_IMP_SYMBOL(daylight) = __daylight();
}

void __cdecl tzset(void)
{
  _tzset();
}

// assert (in wassert.c) produces references to these two functions
int __cdecl fwprintf(FILE *ptr, const wchar_t *fmt, ...)
{
  va_list ap;
  int ret;
  va_start(ap, fmt);
  ret = vfwprintf(ptr, fmt, ap);
  va_end(ap);
  return ret;
}

int __cdecl _snwprintf(wchar_t * restrict _Dest, size_t _Count, const wchar_t * restrict _Format, ...)
{
  va_list ap;
  int ret;
  va_start(ap, _Format);
  ret = vsnwprintf(_Dest, _Count, _Format, ap);
  va_end(ap);
  return ret;
}

// This is called for wchar cases with __USE_MINGW_ANSI_STDIO enabled (where the
// char case just uses fputc). The FILE* is a valid file here, shouldn't be our
// dummy stderr.
int __cdecl __ms_fwprintf(FILE *file, const wchar_t *fmt, ...)
{
  va_list ap;
  int ret;
  va_start(ap, fmt);
  ret = __stdio_common_vfwprintf(UCRTBASE_PRINTF_LEGACY_WIDE_SPECIFIERS, file, fmt, NULL, ap);
  va_end(ap);
  return ret;
}

// Dummy/unused __imp_ wrappers, to make GNU ld not autoexport these symbols.
int __cdecl (*__MINGW_IMP_SYMBOL(__getmainargs))(int *, char ***, char ***, int, _startupinfo *) = __getmainargs;
int __cdecl (*__MINGW_IMP_SYMBOL(__wgetmainargs))(int *, wchar_t ***, wchar_t ***, int, _startupinfo *) = __wgetmainargs;
void __cdecl (*__MINGW_IMP_SYMBOL(_amsg_exit))(int) = _amsg_exit;
unsigned int __cdecl (*__MINGW_IMP_SYMBOL(_get_output_format))(void) = _get_output_format;
void __cdecl (*__MINGW_IMP_SYMBOL(tzset))(void) = tzset;
int __cdecl (*__MINGW_IMP_SYMBOL(fwprintf))(FILE *, const wchar_t *, ...) = fwprintf;
int __cdecl (*__MINGW_IMP_SYMBOL(_snwprintf))(wchar_t *restrict, size_t, const wchar_t *restrict, ...) = _snwprintf;
int __cdecl (*__MINGW_IMP_SYMBOL(__ms_fwprintf))(FILE *, const wchar_t *, ...) = __ms_fwprintf;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
