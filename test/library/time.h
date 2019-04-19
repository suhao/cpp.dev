/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TIME_INCLUDE_H_ 
#define TEST_TIME_INCLUDE_H_ 

#include "types.h"

#include <windows.h>

uint32 Time() { return ::GetTickCount(); }

bool Between(uint32 later, uint32 middle, uint32 earlier) {
    if (earlier <= later) {
        return earlier <= middle && middle <= later;
    } else {
        return !(later < middle && middle < earlier);
    }
}

int32 TimeDiff(uint32 later, uint32 earlier) {
    static uint32 LAST = 0xFFFFFFFF;
    static uint32 HALF = 0x80000000;
    if (Between(earlier + HALF, later, earlier)) {
        if (earlier <= later) return static_cast<long>(later - earlier);
        else return static_cast<long>(later + LAST - earlier + 1);
    } else {
        if (later <= earlier) return -static_cast<long>(earlier - later);
        else return -static_cast<long>(earlier + LAST - later + 1);
    }
}

uint32 StartTime() {
    static const auto start = Time();
    return start;
}

auto ElapsedTime() { return TimeDiff(Time(), StartTime()); }

namespace internal {
static const auto ignore = StartTime();
}

#endif  // !#define (TEST_TIME_INCLUDE_H_ )
