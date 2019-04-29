/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_GDI_OBJECT_INCLUDE_H_
#define DIRECTX_SCOPED_GDI_OBJECT_INCLUDE_H_

#include <windows.h>

namespace win {

// Like ScopedHandle but for GDI objects.
template<class T>
class ScopedGDIObject {
public:
    ScopedGDIObject() : object_(NULL) {}
    explicit ScopedGDIObject(T object) : object_(object) {}

    ~ScopedGDIObject() { Close(); }

    T Get() { return object_; }

    void Set(T object) {
        if (object_ && object != object_) Close();
        object_ = object;
    }

    ScopedGDIObject& operator=(T object) {
        Set(object);
        return *this;
    }

    T release() {
        T object = object_;
        object_ = NULL;
        return object;
    }

    operator T() { return object_; }

private:
    ScopedGDIObject<T>(const ScopedGDIObject<T>&) = delete;
    void operator=(const ScopedGDIObject<T>&) = delete;

    void Close() { if (object_) ::DeleteObject(object_); }

    T object_;
};

// An explicit specialization for HICON because we have to call DestroyIcon()
// instead of DeleteObject() for HICON.
template<>
void ScopedGDIObject<HICON>::Close() {
    if (object_) ::DestroyIcon(object_);
}

// Typedefs for some common use cases.
typedef ScopedGDIObject<HBITMAP> ScopedBitmap;
typedef ScopedGDIObject<HRGN> ScopedRegion;
typedef ScopedGDIObject<HFONT> ScopedHFONT;
typedef ScopedGDIObject<HICON> ScopedHICON;

}  // namespace win

#endif  // DIRECTX_SCOPED_GDI_OBJECT_INCLUDE_H_
