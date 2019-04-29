///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef DRIVER_WINDOWS_DOLBY_INCLUDE_H_
#define DRIVER_WINDOWS_DOLBY_INCLUDE_H_

#include "utils/scoped_com_initializer.h"
#include "utils/dynamic_library.h"

#include "mftransform.h"
#include "Mfapi.h"
#include <Windows.h>
#include <WinDef.h>


// TODO: switch to NTDDI_WIN10_RS3 when _NT_TARGET_VERSION is updated to support RS3
#include <initguid.h>
// {145CD8B4-92F4-4b23-8AE7-E0DF06C2DA95}   MFT_CATEGORY_VIDEO_RENDERER_EFFECT 
DEFINE_GUID(/*MFT_CATEGORY_VIDEO_RENDERER_EFFECT*/MFT_DOLBY_EFFECT,
0x145cd8b4, 0x92f4, 0x4b23, 0x8a, 0xe7, 0xe0, 0xdf, 0x6, 0xc2, 0xda, 0x95);

// MFT_ENUM_VIDEO_RENDERER_EXTENSION_PROFILE {62C56928-9A4E-443b-B9DC-CAC830C24100} 
// Type: VT_VECTOR | VT_LPWSTR 
// MFTEnumEx stores this on the attribute store of the IMFActivate object that  
// MFTEnumEx creates for MFTs that have an associated UWP Manifest containing the tag 
// VideoRendererExtensionProfiles.  This contains a list of all VideoRendererExtensionProfile 
// entries in the VideoRendererExtensionProfiles tag. 
DEFINE_GUID(/*MFT_ENUM_VIDEO_RENDERER_EXTENSION_PROFILE*/MFT_DOLBY_EXTENSION,
0x62c56928, 0x9a4e, 0x443b, 0xb9, 0xdc, 0xca, 0xc8, 0x30, 0xc2, 0x41, 0x0);

DEFINE_GUID(/*MFMediaType_Video*/MFM_DOLBY_VIDEO, 0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

#ifndef FCC
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))
#endif

DEFINE_GUID(/*MFVideoFormat_P010*/MFV_DOLBY_P010, FCC('P010'), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);



int dolby_version() {
    ScopedCOMInitializer com_initializer(ScopedCOMInitializer::kMTA);

    utils::DynamicLibrary library(std::wstring(L"Mfplat.dll"));
    if (!library.is_valid()) return -1;

    auto MFTEnumAPI = library.GetFunctionPointer<HRESULT, GUID, UINT32, const MFT_REGISTER_TYPE_INFO*, const MFT_REGISTER_TYPE_INFO*, IMFActivate***, UINT32*>("MFTEnumEx");
    if (MFTEnumAPI == nullptr) return -2;


    IMFTransform* mf_transform_ = nullptr;
    HRESULT hr = S_OK;

    MFT_REGISTER_TYPE_INFO input_type = { MFM_DOLBY_VIDEO, MFV_DOLBY_P010 };
    IMFActivate **activate_objects = NULL;
    UINT32 activate_objects_count = 0;

    bool  foundMatch = false;
    IMFActivate * ppActivate = nullptr;



    PCWSTR strProfile = L"dvhe.05";
    auto result = MFTEnumAPI(MFT_DOLBY_EFFECT, MFT_ENUM_FLAG_ALL, &input_type, NULL, &activate_objects, &activate_objects_count);


    for (UINT32 index = 0; index < activate_objects_count; index++)
    {
        PROPVARIANT prop;
        if (activate_objects[index] != nullptr &&
            SUCCEEDED(activate_objects[index]->GetItem(MFT_DOLBY_EXTENSION, &prop)))
        {
            for (DWORD strIndex = 0; strIndex < prop.calpstr.cElems; strIndex++)
            {
                if (wcscmp(prop.calpwstr.pElems[strIndex], strProfile) == 0)
                {
                    foundMatch = true;
                    break;
                }
            }
            PropVariantClear(&prop);
        }
        
    if (activate_objects[index] != nullptr)
           activate_objects[index]->Release();
    }

    CoTaskMemFree(activate_objects);

    return 0;
}


#endif // !DRIVER_WINDOWS_DOLBY_INCLUDE_H_