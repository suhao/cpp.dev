/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_WINDOW_PROC_INCLUDE_H_ 
#define DIRECTX_WINDOW_PROC_INCLUDE_H_ 

#include <windows.h>

namespace win {

using WinProcExceptionFilter = int(__cdecl*)(EXCEPTION_POINTERS* info);
WinProcExceptionFilter SetWinProcExceptionFilter(WinProcExceptionFilter filter);
int CallExceptionFilter(EXCEPTION_POINTERS* info);

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
    WNDCLASSEX* class_out);

template <WNDPROC proc>
LRESULT CALLBACK WrappedWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT rv = 0;
    __try {
        rv = proc(hwnd, message, wparam, lparam);
    } __except (CallExceptionFilter(GetExceptionInformation())) {
    }
    return rv;
}

} // namespace win

#endif  // !#define (DIRECTX_WINDOW_PROC_INCLUDE_H_ )