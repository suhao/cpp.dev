/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TEST_MAP_BOUND_INCLUDE_H_ 
#define TEST_TEST_MAP_BOUND_INCLUDE_H_ 

#include <string>
#include <map>

 void map_bound_test() {
    std::map<int, std::wstring> test;
    test[0] = L"aaa";
    test[0] = L"bbb";
    test[1] = L"ccc";
    test[2] = L"ddd";
    test[3] = L"eee";
    test[4] = L"fff";
    test[6] = L"ggg";
    test[7] = L"hhh";
    test[8] = L"iii";

    auto low_3 = test.lower_bound(3);
    auto low_4 = test.lower_bound(4);
    auto low_5 = test.lower_bound(5);
    auto low_6 = test.lower_bound(6);

    auto up_3 = test.upper_bound(3);
    auto up_4 = test.upper_bound(4);
    auto up_5 = test.upper_bound(5);
    auto up_6 = test.upper_bound(6);

    auto lower = test.lower_bound(2);
    auto upper = test.upper_bound(8);

    for (; lower != upper;) {
        if (lower->first % 2 == 0) {
            lower = test.erase(lower);
        }
        else {
            ++lower;
        }
    }

}

#endif  // !#define (TEST_TEST_MAP_BOUND_INCLUDE_H_ )