/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_HGLOBALH_INCLUDE_H_
#define DIRECTX_SCOPED_HGLOBALH_INCLUDE_H_

#include <windows.h>

template<class T>
class ScopedHGlobal {
public:
    explicit ScopedHGlobal(HGLOBAL glob) : glob_(glob) {
        data_ = static_cast<T*>(GlobalLock(glob_));
    }
    ~ScopedHGlobal() { GlobalUnlock(glob_); }

    T* get() { return data_; }

    size_t Size() const { return GlobalSize(glob_); }

    T* operator->() const  {
        assert(data_ != 0);
        return data_;
    }

    T* release() {
        T* data = data_;
        data_ = NULL;
        return data;
    }

private:
    ScopedHGlobal<T>(const ScopedHGlobal<T>&) = delete;
    void operator=(const ScopedHGlobal<T>&) = delete;

    HGLOBAL glob_;
    T* data_ = nullptr;
};

#endif  // !#define (DIRECTX_SCOPED_HGLOBAL.H_INCLUDE_H_ )