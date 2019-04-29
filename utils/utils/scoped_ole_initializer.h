/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef UTILITY_SCOPED_OLE_INITIALIZER_INCLUDE_H_ 
#define UTILITY_SCOPED_OLE_INITIALIZER_INCLUDE_H_ 

#include <ole2.h>

class ScopedOleInitializer {
public:
    explicit ScopedOleInitializer();
    virtual ~ScopedOleInitializer();

private:
#ifndef NDEBUG
    // In debug builds we use this variable to catch a potential bug where a
    // ScopedOleInitializer instance is deleted on a different thread than it
    // was initially created on.  If that ever happens it can have bad
    // consequences and the cause can be tricky to track down.
    DWORD thread_id_;
#endif
    HRESULT hr_;

    ScopedOleInitializer(const ScopedOleInitializer&);
    void operator=(const ScopedOleInitializer&) = delete;
};

#endif  // !#define (UTILITY_SCOPED_OLE_INITIALIZER_INCLUDE_H_ )
