/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_BITMAP_INCLUDE_H_ 
#define DIRECTX_SCOPED_BITMAP_INCLUDE_H_ 

#include <memory>
#include <string>

#include "scoped_gdi_object.h"
#include "scoped_ref_object.h"

class ScopedBitmap
    : public win::ScopedBitmap
    , public AtomicRefObject<ScopedBitmap> {
public:
    explicit ScopedBitmap();
    explicit ScopedBitmap(HANDLE bitmap);
    explicit ScopedBitmap(HBITMAP bitmap);
    explicit ScopedBitmap(const std::string& stream);
    explicit ScopedBitmap(const std::wstring& path);
    virtual ~ScopedBitmap();

public:
    void set(HBITMAP bitmap);
    bool validate();
    SIZE size();
    int width();
    int height();
    LONG bytes();
    std::wstring stringizing();
    std::wstring path();

private:
    SIZE size_ = { 0, 0 };
    LONG bytes_ = 0;
    std::wstring path_;
};



#endif  // !#define (DIRECTX_SCOPED_BITMAP_INCLUDE_H_ )