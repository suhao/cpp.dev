////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////

#include "scoped_com_object.h"


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

HRESULT QueryInterface(const HMODULE& module, const CLSID& clsid, const IID& iid, void** object) {
    using GetClassObjectFunc = HRESULT(*)(const CLSID& clsid, const IID& iid, void** object);
    auto GetClassObject = reinterpret_cast<GetClassObjectFunc>(::GetProcAddress(module, "DllGetClassObject"));
    if (!GetClassObject) return E_FAIL;

    ScopedComObject<IClassFactory> factory;
    auto hr = GetClassObject(clsid, __uuidof(IClassFactory), factory.ReceiveVoid());
    if (FAILED(hr)) return hr;
    return factory->CreateInstance(nullptr, iid, object);
}
