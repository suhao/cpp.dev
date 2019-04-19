/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TEST_STL_INCLUDE_H_ 
#define TEST_TEST_STL_INCLUDE_H_ 


#include <vector>

void test_vector() {

    std::vector<int> v = { 1,2,3,4,5 };
    for (auto it : v) {
        if (it == 5) {
            v.push_back(6);
        }
        if (it == 6) {
            int a = 0;
        }
    }

}


#endif  // !#define (TEST_TEST_STL_INCLUDE_H_ )