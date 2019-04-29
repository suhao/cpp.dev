////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////

#include "scoped_bitmap.h"
#include "scoped_ole_initializer.h"
#include "scoped_handle.h"
#include "scoped_hglobal.h.h"
#include "scoped_com_object.h"

#include <atlimage.h>

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


ScopedBitmap::ScopedBitmap() {

}

ScopedBitmap::ScopedBitmap(HANDLE bitmap) {
    set(static_cast<HBITMAP>(bitmap));
}

ScopedBitmap::ScopedBitmap(HBITMAP bitmap) {
    set(bitmap);
}

ScopedBitmap::ScopedBitmap(const std::string& stream) {
    ScopedOleInitializer ole_initializer;
    CImage init_gdi_plus;
    init_gdi_plus.Load((IStream*)nullptr);
    ScopedGlobalAlloc global_memory(::GlobalAlloc(GHND, stream.length() + 1));
    do {
        if (!global_memory.Get()) break;
        else {
            ScopedHGlobal<void> data(global_memory.Get());
            if (!data.get()) break;
            std::memcpy(data.get(), stream.c_str(), stream.length());
        }

        ScopedComObject<IStream> image_stream;
        if (S_OK != ::CreateStreamOnHGlobal(global_memory, FALSE, image_stream.Receive())) break;

        std::unique_ptr<Gdiplus::Bitmap> image(Gdiplus::Bitmap::FromStream(image_stream, TRUE));
        if (!image || Gdiplus::Ok != image->GetLastStatus()) break;

        HBITMAP handler = nullptr;
        if (Gdiplus::Ok != image->GetHBITMAP(Gdiplus::Color::Black, &handler)) break;
        set(handler);

    } while (false);
}

ScopedBitmap::ScopedBitmap(const std::wstring& path) {
    set([path]()->HBITMAP {
        if (path.empty()) return nullptr;
        CImage image;
        if (FAILED(image.Load(path.c_str()))) return nullptr;
        return image.Detach();
    }());
}

ScopedBitmap::~ScopedBitmap() {

}

void ScopedBitmap::set(HBITMAP bitmap) {
    if (nullptr == bitmap) return;

    BITMAP bitmap_info = { 0 };
    ::GetObject(bitmap, sizeof(BITMAP), &bitmap_info);
    size_.cx = bitmap_info.bmWidth;
    size_.cy = bitmap_info.bmHeight;
    bytes_ = bitmap_info.bmWidthBytes;
    ScopedBitmap::Set(bitmap);
}

bool ScopedBitmap::validate() {
    return (NULL != Get());
}

SIZE ScopedBitmap::size() {
    return size_;
}

int ScopedBitmap::width() {
    return static_cast<int>(size_.cx);
}

int ScopedBitmap::height() {
    return static_cast<int>(size_.cy);
}

LONG ScopedBitmap::bytes() {
    return bytes_;
}

std::wstring ScopedBitmap::stringizing() {
    if (!validate()) return std::wstring();
    TCHAR bitmap[16] = { 0 };
    _stprintf_s(bitmap, 16, L"%u", Get());
    return bitmap;
}

std::wstring ScopedBitmap::path() {
    return path_;
}
