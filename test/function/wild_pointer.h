/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TEST_WILD_POINTER_INCLUDE_H_ 
#define TEST_TEST_WILD_POINTER_INCLUDE_H_ 

#include <string>

void wild_pointer() {
    struct Task {
        ~Task() {
            int a = 0;
        }
        static void f1() {
            Task* task = new Task;
            delete task;
            task->f2();
        }
        void f2() {
            int a = 10;
            try {
                member = 100;
            } catch (...) {
            	
            }

        }
        int member = 0;
    };
    Task::f1();
}

namespace wild {
class Task {
public:
    Task() {
        p = new int(1);
    }
    ~Task() { /*delete p;*/ }
    static void f1() {
        Task* task = new Task;
        delete task;
        task->f2();
    }
    void f2() {
        int a = 10;
        auto var = var1;
        var = var2;
        //auto var666 = var3;
        var = *p;
    }
    static int var1;
    int var2 = 666;
    int* p = nullptr;
    std::string var3 = "task";
};
int Task::var1 = 555;
}

void wild_pointer_static_member_variable() {
    wild::Task::f1();
}


#endif  // !#define (TEST_TEST_WILD_POINTER_INCLUDE_H_ )