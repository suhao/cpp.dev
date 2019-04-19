/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TEST_AES_INCLUDE_H_ 
#define TEST_TEST_AES_INCLUDE_H_ 

#include "openssl/crypto/aes/aes.h"


#include <string>
#include <memory>

namespace aes {


namespace internal {

void MakeVector(const std::string& key, unsigned char* aes, int length) {
    if (!aes) return;
    auto quotient = std::div(key.length(), length);
    for (int i = 0; i < quotient.quot; ++i) {
        for (int j = 0; j < length; ++j) {
            aes[j] |= key.at(i * length + j);
        }
    }
}
}


bool encrypt(const std::string& key, std::string dec, std::string& enc, int bits /*= 128*/) {
    if (key.empty() || dec.empty()) return false;

    auto quotient = std::div(dec.length(), AES_BLOCK_SIZE);
    dec.append(AES_BLOCK_SIZE - quotient.rem, quotient.rem);

    AES_KEY aes_key = { 0 };
    if (0 != AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), bits, &aes_key)) {
        return false;
    }

    unsigned char aes_vector[AES_BLOCK_SIZE] = { 0 };
    internal::MakeVector(key, aes_vector, AES_BLOCK_SIZE);

    std::unique_ptr<unsigned char[]> value(new unsigned char[dec.length() + 1]);
    memset(value.get(), 0, dec.length() + 1);
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(dec.c_str()), value.get(), dec.length(), &aes_key, aes_vector, AES_ENCRYPT);
    enc.assign(reinterpret_cast<char*>(value.get()), dec.length());
    return true;
}

bool decrypt(const std::string& key, const std::string& enc, std::string& dec, int bits /*= 128*/) {
    if (key.empty() || enc.empty() || (enc.length() % AES_BLOCK_SIZE) != 0) return false;


    AES_KEY aes_key = { 0 };
    if (0 != AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), bits, &aes_key)) {
        return false;
    }

    unsigned char aes_vector[AES_BLOCK_SIZE] = { 0 };
    internal::MakeVector(key, aes_vector, AES_BLOCK_SIZE);

    std::unique_ptr<unsigned char[]> value(new unsigned char[enc.length() + 1]);
    memset(value.get(), 0, enc.length() + 1);
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(enc.c_str()), value.get(), enc.length(), &aes_key, aes_vector, AES_DECRYPT);

    std::string decrypt(reinterpret_cast<char*>(value.get()), enc.length());

    int rem = static_cast<int>(decrypt.back());
    if (rem > AES_BLOCK_SIZE || rem < 0) {
        return false;
    }
    dec = decrypt.substr(0, enc.length() + rem - AES_BLOCK_SIZE);
    return true;
}

}

#endif  // !#define (TEST_TEST_AES_INCLUDE_H_ )