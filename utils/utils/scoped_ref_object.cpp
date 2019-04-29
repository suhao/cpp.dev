////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////

#include "scoped_ref_object.h"

#include <windows.h>

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

namespace {

inline auto AtomicIncrement(volatile int* ptr, int increment) {
    return InterlockedExchangeAdd(
        reinterpret_cast<volatile LONG*>(ptr),
        static_cast<LONG>(increment)) + increment;
}

inline bool AtomicIsOne(volatile int* ptr) {
    auto value = *ptr;
    return 1 == value;
}

}

subtle::RefCounted::RefCounted() {

}

subtle::RefCounted::~RefCounted() {

}

void subtle::RefCounted::AddRef() const {
    ++ref_count_;
}

bool subtle::RefCounted::Release() const {
    return (--ref_count_ == 0);
}

bool subtle::RefCounted::OneRef() const {
    return (ref_count_ == 1);
}

subtle::AtomicRefCounted::AtomicRefCounted() {

}

subtle::AtomicRefCounted::~AtomicRefCounted() {

}

void subtle::AtomicRefCounted::AddRef() const {
    AtomicIncrement(&ref_count_, 1);
}

bool subtle::AtomicRefCounted::Release() const {
    return (0 == AtomicIncrement(&ref_count_, -1));
}

bool subtle::AtomicRefCounted::OneRef() const {
    return AtomicIsOne(&ref_count_);
}
