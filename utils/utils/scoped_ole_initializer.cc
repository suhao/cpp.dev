////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////
#include "scoped_ole_initializer.h"
#include <Windows.h>

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

ScopedOleInitializer::ScopedOleInitializer()
    :
#ifndef NDEBUG
      // Using the windows API directly to avoid dependency on platform_thread.
      thread_id_(::GetCurrentThreadId()),
#endif
      hr_(::OleInitialize(NULL)) {
#ifndef NDEBUG
    if (hr_ == S_FALSE) {
        // LOG(ERROR) << "Multiple OleInitialize() calls for thread " << thread_id_;
    } else {
        // DCHECK_NE(OLE_E_WRONGCOMPOBJ, hr_) << "Incompatible DLLs on machine";
        // DCHECK_NE(RPC_E_CHANGED_MODE, hr_) << "Invalid COM thread model change";
    }
#endif
}

ScopedOleInitializer::~ScopedOleInitializer() {
#ifndef NDEBUG
    // DCHECK_EQ(thread_id_, ::GetCurrentThreadId());
#endif
    if (SUCCEEDED(hr_)) {
        ::OleUninitialize();
    }
}
