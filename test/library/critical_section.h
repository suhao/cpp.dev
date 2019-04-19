/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_CPU_INCLUDE_H_ 
#define TEST_CPU_INCLUDE_H_ 

#include <Windows.h>

class Section {
public:
    Section() { ::InitializeCriticalSection(&section_); }
    virtual ~Section() { ::DeleteCriticalSection(&section_); }
    void lock() { ::EnterCriticalSection(&section_); }
    void unlock() { ::LeaveCriticalSection(&section_); }
private:
    CRITICAL_SECTION section_;
};

class ScopedSection {
public:
    ScopedSection(Section* section) : section_(section) { section->lock(); }
    virtual ~ScopedSection() { section_->unlock(); }
private:
    Section * section_;
};


#endif  // !#define (TEST_CPU_INCLUDE_H_ )