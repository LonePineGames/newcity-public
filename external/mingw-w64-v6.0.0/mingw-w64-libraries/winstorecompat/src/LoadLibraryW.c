/*
    Copyright (c) 2013-2016 mingw-w64 project

    Contributing authors: Jean-Baptiste Kempf

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#define _WIN32_WINNT 0x602 /* LoadPackagedLibrary is Win8 APP Family */

#define LoadLibraryW __LoadLibraryW
#include <windef.h>
#include <winbase.h>
#undef LoadLibraryW

HMODULE WINAPI LoadLibraryW(LPCWSTR lpFileName)
{
    return LoadPackagedLibrary(lpFileName, 0);
}

#ifdef _X86_
HMODULE (WINAPI *__MINGW_IMP_SYMBOL(LoadLibraryW))(LPCWSTR lpFileName) asm("__imp__LoadLibraryW@4") = LoadLibraryW;
#else
HMODULE (WINAPI *__MINGW_IMP_SYMBOL(LoadLibraryW))(LPCWSTR lpFileName) asm("__imp_LoadLibraryW") = LoadLibraryW;
#endif
