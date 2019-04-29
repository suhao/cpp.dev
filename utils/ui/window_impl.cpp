////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////

#include "window_impl.h"

#include <list>
#include <memory>
#include <mutex>

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

namespace ui {

const wchar_t* const Window::kBaseClassName = L"DirectX_Win_";
static const DWORD kWindowDefaultChildStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
static const DWORD kWindowDefaultStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
static const DWORD kWindowDefaultExStyle = 0;

void WindowTraits::operator()(Window* x) {
    if (nullptr == x) return;
    x->Close();
    delete x;
}

WNDPROC SetWindowProc(HWND hwnd, WNDPROC proc) {
    WNDPROC oldwindow_proc =
        reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC));
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc));
    return oldwindow_proc;
}

HWND GetWindowToParentTo(bool get_real_hwnd) {
    return get_real_hwnd ? ::GetDesktopWindow() : HWND_DESKTOP;
}

void* SetWindowUserData(HWND hwnd, void* user_data) {
    return
        reinterpret_cast<void*>(SetWindowLongPtr(hwnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(user_data)));
}

void* GetWindowUserData(HWND hwnd) {
    DWORD process_id = 0;
    GetWindowThreadProcessId(hwnd, &process_id);
    // A window outside the current process needs to be ignored.
    if (process_id != ::GetCurrentProcessId())
        return NULL;
    return reinterpret_cast<void*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

RECT GetWindowBoundsForClientBounds(DWORD style, DWORD ex_style, const RECT& bounds) {
    RECT wr = bounds;
    AdjustWindowRectEx(&wr, style, FALSE, ex_style);

    // Make sure to keep the window onscreen, as AdjustWindowRectEx() may have
    // moved part of it offscreen.
    if (wr.left < 0) {
        wr.right = wr.right - wr.left;
        wr.left = 0;
    }

    if (wr.top < 0) {
        wr.bottom = wr.bottom - wr.top;
        wr.top = 0;
    }
    return wr;
}

} // namespace ui


struct ClassInfo {
    UINT style;
    HICON icon;
    HICON small_icon;

    ClassInfo(int style, HICON icon, HICON small_icon)
        : style(style), icon(icon), small_icon(small_icon) {}

    bool Equals(const ClassInfo& other) const {
        return (other.style == style && other.icon == icon && other.small_icon == small_icon);
    }
};

class ClassRegistrar {
public:
    ~ClassRegistrar() {}

    static ClassRegistrar* GetInstance();

    void UnregisterClasses();

    // Returns the atom identifying the class matching |class_info|,
    // creating and registering a new class if the class is not yet known.
    ATOM RetrieveClassAtom(const ClassInfo& class_info);

private:
    ClassRegistrar(const ClassRegistrar&) = delete;
    void operator=(const ClassRegistrar&) = delete;

    struct RegisteredClass {
        RegisteredClass(const ClassInfo& info, const std::wstring& name, ATOM atom, HINSTANCE instance)
            : info(info)
            , name(name)
            , atom(atom)
            , instance(instance) {
        }
        ClassInfo info;
        std::wstring name;
        ATOM atom;
        HMODULE instance;
    };

    ClassRegistrar() {}

    using RegisteredClasses = std::list<RegisteredClass>;
    RegisteredClasses registered_classes_;
    int registered_count_ = 0;
    std::mutex mutex_;
};

// static
ClassRegistrar* ClassRegistrar::GetInstance() {
    static ClassRegistrar* registrar = nullptr;
    if (!registrar) {
        std::unique_ptr<ClassRegistrar> new_registrar(new ClassRegistrar());
        if (InterlockedCompareExchangePointer(
            reinterpret_cast<PVOID*>(&registrar), new_registrar.get(), NULL)) {
        } else {
            new_registrar.release();
        }
    }
    return registrar;
}

void ClassRegistrar::UnregisterClasses() {
    for (auto i = registered_classes_.begin();
        i != registered_classes_.end();) {
        if (UnregisterClass(MAKEINTATOM(i->atom), i->instance)) {
            i = registered_classes_.erase(i);
            continue;
        } else {
            auto code = GetLastError();
        }
        ++i;
    }
}

ATOM ClassRegistrar::RetrieveClassAtom(const ClassInfo& class_info) {
    mutex_.lock();
    for (RegisteredClasses::const_iterator i = registered_classes_.begin();
        i != registered_classes_.end(); ++i) {
        if (class_info.Equals(i->info))
            return i->atom;
    }

    // No class found, need to register one.
    auto name = std::wstring(ui::Window::kBaseClassName).append(1, registered_count_++);

    WNDCLASSEX window_class;
    win::InitializeWindowClass(name.c_str(), &win::WrappedWindowProc<ui::Window::WndProc>,
        class_info.style, 0, 0, NULL,
        reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)), NULL,
        class_info.icon, class_info.small_icon, &window_class);
    HMODULE instance = window_class.hInstance;
    ATOM atom = RegisterClassEx(&window_class);
    auto code = GetLastError();
    registered_classes_.push_back(RegisteredClass(class_info, name, atom, instance));
    return atom;
}

namespace ui {

// static
void RunLoop() {
    MSG msg;
    for (;;) {
        if (FALSE == ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) continue;
        if (WM_QUIT == msg.message) return;
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

// static
void RegisterClassesAtExit() {
    std::atexit([]() {
        ClassRegistrar::GetInstance()->UnregisterClasses();
    });
}

Window::Window(WindowDelegate* delegate)
    : window_style_(0)
    , window_ex_style_(kWindowDefaultExStyle)
    , class_style_(CS_DBLCLKS)
    , hwnd_(NULL)
    , delegate_(delegate) {
}

Window::~Window() {
    ClearUserData();
}

void Window::Init(HWND parent, const RECT& bounds) {
    if (window_style_ == 0)
        window_style_ = parent ? kWindowDefaultChildStyle : kWindowDefaultStyle;

    if (parent == HWND_DESKTOP) {
        parent = GetWindowToParentTo(false);
    } else if (parent == ::GetDesktopWindow()) {
        // Any type of window can have the "Desktop Window" as their parent.
        parent = GetWindowToParentTo(true);
    } else if (parent != HWND_MESSAGE) {
    }

    int x, y, width, height;
    if (TRUE == ::IsRectEmpty(&bounds)) {
        x = y = width = height = CW_USEDEFAULT;
    } else {
        auto rect = GetWindowBoundsForClientBounds(window_style(), window_ex_style(), bounds);
        x = rect.left;
        y = rect.top;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    ATOM atom = GetWindowClassAtom();
    HWND hwnd = CreateWindowEx(window_ex_style_,
        reinterpret_cast<wchar_t*>(atom), NULL,
        window_style_, x, y, width, height,
        parent, NULL, NULL, this);
    if (hwnd && (window_style_ & WS_CAPTION)) {
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
    }

    if (!hwnd_ && GetLastError() == 0) {
        WNDCLASSEX class_info;
        memset(&class_info, 0, sizeof(WNDCLASSEX));
        class_info.cbSize = sizeof(WNDCLASSEX);
        BOOL got_class = GetClassInfoEx(GetModuleHandle(NULL),
            reinterpret_cast<wchar_t*>(atom),
            &class_info);
        bool procs_match = got_class && class_info.lpfnWndProc == win::WrappedWindowProc<&Window::WndProc>;
    }

    if (!hwnd) {
        switch (GetLastError()) {
        case ERROR_NOT_ENOUGH_MEMORY:
            break;
        case ERROR_ACCESS_DENIED:
            break;
        default:
            break;
        }
    }
}

void Window::Close() {
    if (TRUE == ::IsWindow(hwnd_)) {
        ::DestroyWindow(hwnd_);
    }
}

HICON Window::GetDefaultWindowIcon() const {
    return nullptr;
}

HICON Window::GetSmallWindowIcon() const {
    return nullptr;
}

void Window::set_bounds(const RECT& bounds) {
    auto window_bounds = GetWindowBoundsForClientBounds(
        GetWindowLong(hwnd(), GWL_STYLE),
        GetWindowLong(hwnd(), GWL_EXSTYLE),
        bounds);
    unsigned int flags = SWP_NOREPOSITION;
    if (!::IsWindowVisible(hwnd()))
        flags |= SWP_NOACTIVATE;
    ::SetWindowPos(hwnd(), NULL, window_bounds.left, window_bounds.top,
        window_bounds.right - window_bounds.left, window_bounds.bottom - window_bounds.top, flags);
}

RECT Window::bounds() {
    RECT cr = { 0 };
    ::GetClientRect(hwnd(), &cr);
    return cr;
}

void Window::FireEvent(const WindowDelegate::Dispatch& dispatcher) {
    dispatcher_ = dispatcher;
}

void Window::SetTitle(const std::wstring& title) {
    if (FALSE == ::IsWindow(hwnd_)) return;
    ::SetWindowText(hwnd_, title.c_str());
}

void Window::Show(int cmd_show/* = SW_SHOWNORMAL*/) {
    if (FALSE == ::IsWindow(hwnd_)) return;
    ::ShowWindow(hwnd_, cmd_show);
}

void Window::Hide() {
    if (FALSE == ::IsWindow(hwnd_)) return;
    ::ShowWindow(hwnd_, SW_HIDE);
}

void Window::SetCapture() {
    if (::GetCapture() != hwnd())
        ::SetCapture(hwnd());
}

void Window::ReleaseCapture() {
    if (::GetCapture() == hwnd())
        ::ReleaseCapture();
}

LRESULT Window::OnWndProc(UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    HWND hwnd = hwnd_;
    if (message == WM_NCDESTROY)
        hwnd_ = NULL;

    if (!ProcessWindowMessage(hwnd, message, w_param, l_param, result))
        result = ::DefWindowProc(hwnd, message, w_param, l_param);

    return result;
}

BOOL Window::ProcessWindowMessage(HWND window, UINT message, WPARAM w_param, LPARAM l_param, LRESULT& result, DWORD msg_map_id /*= 0*/) {
    switch (message) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        return TRUE;
    } break;
    default:
        break;
    }

    if (dispatcher_ && dispatcher_(window, message, w_param, l_param)) { return TRUE; }
    if (!delegate_) return FALSE;
    return delegate_->DispatchEvent(hwnd(), message, w_param, l_param);
}

void Window::ClearUserData() {
    if (::IsWindow(hwnd_)) SetWindowUserData(hwnd_, NULL);
}

// static
LRESULT CALLBACK Window::WndProc(HWND hwnd,
    UINT message,
    WPARAM w_param,
    LPARAM l_param) {
    Window* window = nullptr;
    if (message == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(l_param);
        window = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowUserData(hwnd, window);
        window->hwnd_ = hwnd;
    } else {
        window = reinterpret_cast<Window*>(GetWindowUserData(hwnd));
    }

    if (!window)
        return 0;

    return window->OnWndProc(message, w_param, l_param);
}

ATOM Window::GetWindowClassAtom() {
    HICON icon = GetDefaultWindowIcon();
    HICON small_icon = GetSmallWindowIcon();
    ClassInfo class_info(initial_class_style(), icon, small_icon);
    return ClassRegistrar::GetInstance()->RetrieveClassAtom(class_info);
}


} // namespace ui