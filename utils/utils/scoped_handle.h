/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_HANDLE_INCLUDE_H_ 
#define DIRECTX_SCOPED_HANDLE_INCLUDE_H_ 

#include <windows.h>

template <class Traits>
class GenericScopedHandle {
public:
    typedef typename Traits::Handle Handle;
    GenericScopedHandle() : handle_(Traits::NullHandle()) {}

    explicit GenericScopedHandle(Handle handle) : handle_(Traits::NullHandle()) {
        Set(handle);
    }
    ~GenericScopedHandle() {
        Close();
    }

    bool IsValid() const { return Traits::IsHandleValid(handle_); }

    void Set(Handle handle) {
        if (handle_ == handle) return;
        Close();
        if (!Traits::IsHandleValid(handle)) return;
        handle_ = handle;
    }

    Handle Get() const { return handle_; }

    operator Handle() const { return handle_; }

    Handle Release() {
        Handle temp = handle_;
        handle_ = Traits::NullHandle();
        return temp;
    }

    void Close() {
        if (!Traits::IsHandleValid(handle_)) return;
        handle_ = Traits::NullHandle();
    }

private:
    Handle handle_;
};

class HandleTraits {
public:
    typedef HANDLE Handle;
    static bool CloseHandle(HANDLE handle) { return ::CloseHandle(handle) != FALSE; }
    static bool IsHandleValid(HANDLE handle) { return handle != NULL && handle != INVALID_HANDLE_VALUE; }
    static HANDLE NullHandle() { return NULL; }

private:
    HandleTraits() = delete;
    HandleTraits(const HandleTraits&) = delete;
    void operator=(const HandleTraits&) = delete;
};

class HGlobalTraits {
public:
    typedef HGLOBAL Handle;
    static bool CloseHandle(Handle handle) { return nullptr == ::GlobalFree(handle); }
    static bool IsHandleValid(Handle handle) { return handle != nullptr; }
    static Handle NullHandle() { return nullptr; }
private:
    HGlobalTraits() = delete;
    HGlobalTraits(const HGlobalTraits&) = delete;
    void operator=(const HGlobalTraits&) = delete;
};

typedef GenericScopedHandle<HandleTraits> ScopedHandle;
typedef GenericScopedHandle<HGlobalTraits> ScopedGlobalAlloc;

#endif  // !#define (DIRECTX_SCOPED_HANDLE_INCLUDE_H_ )
