/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_WINDOW_DELEGATE_INCLUDE_H_ 
#define DIRECTX_WINDOW_DELEGATE_INCLUDE_H_ 

#include <windows.h>
#include <functional>

namespace ui {

enum WindowState {
    WINDOW_STATE_UNKNOWN,
    WINDOW_STATE_MAXIMIZED,
    WINDOW_STATE_MINIMIZED,
    WINDOW_STATE_NORMAL,
    WINDOW_STATE_FULLSCREEN,
};

class WindowDelegate {
public:
    virtual ~WindowDelegate() {}

    using Dispatch = std::function<BOOL(HWND, UINT, WPARAM, LPARAM)>;
    virtual BOOL DispatchEvent(HWND window, UINT message, WPARAM w_param, LPARAM l_param) = 0;

    virtual void OnWindowStateChanged(WindowState new_state) = 0;
};

}

#endif  // !#define (DIRECTX_WINDOW_DELEGATE_INCLUDE_H_ )