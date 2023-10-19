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

#define _WIN32_WINNT 0x501 /* SetFilePointerEx is XP+ */

#define SetFilePointer __SetFilePointer
#include <windef.h>
#include <windows.h>
#undef SetFilePointer

DWORD WINAPI SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    LARGE_INTEGER liDistanceToMove, newpos;

    if (lpDistanceToMoveHigh)
    {    
        liDistanceToMove.u.LowPart  = lDistanceToMove;
        liDistanceToMove.u.HighPart = *lpDistanceToMoveHigh;
    }    
    else liDistanceToMove.QuadPart = lDistanceToMove;

    if (!SetFilePointerEx( hFile, liDistanceToMove, &newpos, dwMoveMethod ))
        return INVALID_SET_FILE_POINTER;

    if (lpDistanceToMoveHigh)
        *lpDistanceToMoveHigh = newpos.u.HighPart;
    else if (newpos.u.HighPart > 0 )
    {
        return INVALID_SET_FILE_POINTER;
    }

    /* INVALID_SET_FILE_POINTER is a valid value for the low-order DWORD of the new file pointer */
    if (newpos.u.LowPart == INVALID_SET_FILE_POINTER)
        SetLastError( 0 ); 
    return newpos.u.LowPart;
}

#ifdef _X86_
DWORD (WINAPI *__MINGW_IMP_SYMBOL(SetFilePointer))(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) asm("__imp__SetFilePointer@16") = SetFilePointer;
#else
DWORD (WINAPI *__MINGW_IMP_SYMBOL(SetFilePointer))(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) asm("__imp_SetFilePointer") = SetFilePointer;
#endif
