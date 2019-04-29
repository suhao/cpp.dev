/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_WINDOW_IMPL_INCLUDE_H_ 
#define DIRECTX_WINDOW_IMPL_INCLUDE_H_ 

#include <windows.h>
#include <string>
#include <memory>

#include "window_delegate.h"
#include "window_msg_util.h"

class ClassRegistrar;

namespace ui {

void RegisterClassesAtExit();
void RunLoop();

class Window;
struct WindowTraits { void operator()(Window* x); };
using ScopedWindow = std::unique_ptr<Window, WindowTraits>;

class Window : public ui::MessageMapInterface {
public:
    Window(WindowDelegate* delegate);
    virtual ~Window();

    void Init(HWND parent, const RECT& bounds);
    void Close();

    virtual HICON GetDefaultWindowIcon() const;
    virtual HICON GetSmallWindowIcon() const;

    HWND hwnd() const { return hwnd_; }

public:
    void set_window_style(DWORD style) { window_style_ = style; }
    DWORD window_style() const { return window_style_; }

    void set_window_ex_style(DWORD style) { window_ex_style_ = style; }
    DWORD window_ex_style() const { return window_ex_style_; }

    void set_initial_class_style(UINT class_style) { class_style_ = class_style; }
    UINT initial_class_style() const { return class_style_; }

    void set_bounds(const RECT& bounds);
    RECT bounds();

    void FireEvent(const WindowDelegate::Dispatch& dispatcher);
    void SetTitle(const std::wstring& title);
    void Show(int cmd_show = SW_SHOWNORMAL);
    void Hide();
    void SetCapture();
    void ReleaseCapture();
    void ToggleFullscreen() {}
    void Maximize() {}
    void Minimize() {}
    void Restore() {}


protected:
    virtual LRESULT OnWndProc(UINT message, WPARAM w_param, LPARAM l_param);
    virtual BOOL ProcessWindowMessage(HWND window, UINT message, WPARAM w_param, LPARAM l_param, LRESULT& result, DWORD msg_map_id = 0) override;

    void ClearUserData();

private:
    friend class ClassRegistrar;
    Window(const Window&) = delete;
    void operator=(const Window&) = delete;

    static LRESULT CALLBACK WndProc(HWND window,
        UINT message,
        WPARAM w_param,
        LPARAM l_param);

    ATOM GetWindowClassAtom();

    static const wchar_t* const kBaseClassName;
    DWORD window_style_;
    DWORD window_ex_style_;
    UINT class_style_;
    HWND hwnd_;
    WindowDelegate* delegate_ = nullptr;
    WindowDelegate::Dispatch dispatcher_;
};

} // namespace ui

#endif  // !#define (DIRECTX_WINDOW_IMPL_INCLUDE_H_ )