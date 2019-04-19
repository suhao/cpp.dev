/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_FILE_STREAM_INCLUDE_H_ 
#define TEST_FILE_STREAM_INCLUDE_H_ 

#include <Windows.h>
#include <string>

class File {
public:
    File() {}
    virtual ~File() { close(); }

    int open(std::string path, std::string mode) { 
        file_ = fopen(path.c_str(), mode.c_str());
        return 0;
    }
    void close() {
        if (file_ == nullptr) return;
        fclose(file_);
    }
    size_t size() {
        if (!file_) return 0;
        fseek(file_, 0, SEEK_END);
        return ftell(file_);
    }
    size_t write(std::string& data) {
        if (!file_) return 0;
        fwrite(data.c_str(), sizeof(char), data.length(), file_);
    }
    size_t read(std::string data) {
        if (!file_) return 0;
        fread((void*)data[0], sizeof(char), data.length(), file_);
    }
    void seek(size_t pos) { fseek(file_, pos, SEEK_SET); }
    void flush() { fflush(file_); }
    bool end() { feof(file_); }

private:
    FILE * file_ = nullptr;

};

#endif  // !#define (TEST_FILE_STREAM_INCLUDE_H_ )

