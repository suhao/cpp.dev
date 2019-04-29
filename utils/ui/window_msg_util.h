/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_WINDOW_MSG_UTIL_INCLUDE_H_ 
#define DIRECTX_WINDOW_MSG_UTIL_INCLUDE_H_ 

#include <Windows.h>

// Based on atlcrack.h(WTL) to realize the BEGIN_MSG_MAP in the furture.

namespace ui {

class MessageMapInterface {
public:
    virtual ~MessageMapInterface() {}
    virtual BOOL ProcessWindowMessage(
        HWND window,
        UINT message,
        WPARAM w_param,
        LPARAM l_param,
        LRESULT& result,
        DWORD msg_map_id = 0) = 0;
};

} // namespace ui

#endif  // !#define (DIRECTX_WINDOW_MSG_UTIL_INCLUDE_H_ )