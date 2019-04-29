/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_HDC_INCLUDE_H_ 
#define DIRECTX_SCOPED_HDC_INCLUDE_H_ 

#include <windows.h>
#include "scoped_handle.h"

class ScopedGetDC {
public:
    explicit ScopedGetDC(HWND hwnd)
        : hwnd_(hwnd)
        , hdc_(GetDC(hwnd)) {
    }
    ~ScopedGetDC() { if (hdc_) ReleaseDC(hwnd_, hdc_); }
    operator HDC() { return hdc_; }

private:
    ScopedGetDC(const ScopedGetDC&) = delete;
    void operator=(const ScopedGetDC&) = delete;
    HWND hwnd_;
    HDC hdc_;
};

class CreateDCTraits {
public:
    typedef HDC Handle;
    static bool CloseHandle(HDC handle) { return ::DeleteDC(handle) != FALSE; }
    static bool IsHandleValid(HDC handle) { return handle != NULL; }
    static HDC NullHandle() { return NULL; }

private:
    CreateDCTraits() = delete;
    CreateDCTraits(const CreateDCTraits&) = delete;
    void operator=(const CreateDCTraits&) = delete;
};

typedef GenericScopedHandle<CreateDCTraits> ScopedCreateDC;

#endif  // !#define (DIRECTX_SCOPED_HDC_INCLUDE_H_ )
