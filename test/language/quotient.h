/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TEST_QUOTIENT_INCLUDE_H_ 
#define TEST_TEST_QUOTIENT_INCLUDE_H_ 

#include <stdlib.h>
#include <cstdlib>

void test_quotient() {
    auto quotient = std::div(16, 16);
    auto q1 = std::div(32, 16);
    auto q2 = std::div(0, 16);
    auto q3 = std::div(31, 16);
}

#endif  // !#define (TEST_TEST_QUOTIENT_INCLUDE_H_ )