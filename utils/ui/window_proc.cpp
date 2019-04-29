////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////

#include "window_proc.h"


#if defined(COMPILER_MSVC)
// We usually use the _CrtDumpMemoryLeaks() with the DEBUGER and CRT library to
// check a memory leak.
#if defined(_DEBUG) && _MSC_VER > 1000 // VC++ DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define  DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#if defined(DEBUG_NEW)
#define new DEBUG_NEW
#endif // DEBUG_NEW
#endif // VC++ DEBUG
#endif // defined(COMPILER_MSVC)
#include <assert.h>

namespace {

win::WinProcExceptionFilter g_exception_filter = nullptr;

HMODULE GetModuleFromWndProc(WNDPROC window_proc) {
    HMODULE instance = NULL;
    // Converting a pointer-to-function to a void* is undefined behavior, but
    // Windows (and POSIX) APIs require it to work.
    void* address = reinterpret_cast<void*>(window_proc);
    if (!::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<char*>(address), &instance)) {
        assert(false);
    }
    return instance;
}

}  // namespace.

namespace win {

WinProcExceptionFilter SetWinProcExceptionFilter(WinProcExceptionFilter filter) {
  auto rv = ::InterlockedExchange(reinterpret_cast<volatile LONG*>(&g_exception_filter), reinterpret_cast<LONG>(filter));
  return reinterpret_cast<WinProcExceptionFilter>(rv);
}

int CallExceptionFilter(EXCEPTION_POINTERS* info) {
    return g_exception_filter ? g_exception_filter(info) : EXCEPTION_CONTINUE_SEARCH;
}

void InitializeWindowClass(
    const wchar_t* class_name,
    WNDPROC window_proc,
    UINT style,
    int class_extra,
    int window_extra,
    HCURSOR cursor,
    HBRUSH background,
    const wchar_t* menu_name,
    HICON large_icon,
    HICON small_icon,
    WNDCLASSEX* class_out) {
    assert(class_out != nullptr);

    class_out->cbSize = sizeof(WNDCLASSEX);
    class_out->style = style;
    class_out->lpfnWndProc = window_proc;
    class_out->cbClsExtra = class_extra;
    class_out->cbWndExtra = window_extra;
    // RegisterClassEx uses a handle of the module containing the window procedure
    // to distinguish identically named classes registered in different modules.
    class_out->hInstance = GetModuleFromWndProc(window_proc);
    class_out->hIcon = large_icon;
    class_out->hCursor = cursor;
    class_out->hbrBackground = background;
    class_out->lpszMenuName = menu_name;
    class_out->lpszClassName = class_name;
    class_out->hIconSm = small_icon;

    // Check if |window_proc| is valid.
    assert(class_out->hInstance != NULL);
}

} // namespace win