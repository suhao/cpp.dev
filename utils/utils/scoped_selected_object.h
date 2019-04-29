/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_SELECTED_OBJECT_INCLUDE_H_ 
#define DIRECTX_SCOPED_SELECTED_OBJECT_INCLUDE_H_ 

#include <Windows.h>
#include "scoped_ref_object.h"

template<class T>
class ScopedSelectedObject : public AtomicRefObject<ScopedSelectedObject<T>> {
public:
    explicit ScopedSelectedObject(HDC hdc, T object) : hdc_(hdc) {
        if (NULL != hdc) {
            object_ = static_cast<T>(::SelectObject(hdc, static_cast<HGDIOBJ>(object)));
        }
    }

    ~ScopedSelectedObject() {
        Close();
    }

private:
    ScopedSelectedObject<T>(const ScopedSelectedObject<T>&) = delete;
    void operator=(const ScopedSelectedObject<T>&) = delete;

    void Close() {
        if (object_ && hdc_) ::SelectObject(hdc_, static_cast<HGDIOBJ>(object_));
        hdc_ = NULL;
        object_ = NULL;
    }

    HDC hdc_;
    T object_;
};

// Typedefs for some common use cases.
typedef ScopedSelectedObject<HBITMAP> ScopedSelectedBitmap;
typedef ScopedSelectedObject<HRGN> ScopedSelectedRegion;
typedef ScopedSelectedObject<HFONT> ScopedSelectedHFONT;


#endif  // !#define (DIRECTX_SCOPED_SELECTED_OBJECT_INCLUDE_H_ )