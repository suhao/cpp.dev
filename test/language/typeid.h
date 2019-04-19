///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef CPP_TYPEID_INCLUDE_H_
#define CPP_TYPEID_INCLUDE_H_

#include <iostream>
#include <stdio.h>

namespace cpp {

class Base {
public:
    virtual void func() {
        std::cout << "this:" << typeid(this).name() << std::endl;
        std::cout << "*this:" << typeid(*this).name() << std::endl;

        std::cout << this->m << std::endl;
        std::cout << (*this).m << std::endl;
    }
    int m = 1;
};

class Child : public Base {
public:
    int m = 2;
};


void test_typeid() {
    Child b;
    b.func();
}

}

#endif // !CPP_TYPEID_INCLUDE_H_