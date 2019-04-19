/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_BASE64_INCLUDE_H_ 
#define TEST_BASE64_INCLUDE_H_ 

#include <string>

#include "atlenc.h"

#define CNT 10*1024

class encoder {
public:
    encoder() {}
    virtual ~encoder() {}

    const char* encode(char* in, std::string* out) {
        std::string temp;
        auto  count = Base64EncodeGetRequiredLength(strlen(in));
        temp.resize(count);
        Base64Encode((unsigned char*)in, strlen(in), (char*)temp.c_str(), &count);
        out->swap(temp);
        return out->c_str();

    }
    const char* decode(char* in, std::string* out) {
        std::string temp;
        auto count = Base64DecodeGetRequiredLength(strlen(in));
        temp.resize(count);
        Base64Decode(in, strlen(in), (unsigned char*)temp.c_str(), &count);
        out->swap(temp);
        return out->c_str();
    }
};


#endif  // !#define (TEST_BASE64_INCLUDE_H_ )