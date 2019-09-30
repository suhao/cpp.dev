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
#include <list>

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

void test_list() {
    std::list<int> list1 = { 8,9,4,5,0,2,9 };
    std::list<int> list2 = { 2,3,5,4,1 };
    list1.splice(list1.end(), list2);
    list1.merge(list2);
    int a = 0;
}


#endif  // !#define (TEST_TEST_STL_INCLUDE_H_ )