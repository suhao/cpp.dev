/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef PROJECT_BYTE_INCLUDE_H_ 
#define PROJECT_BYTE_INCLUDE_H_ 

#include "types.h"

#ifdef WIN32
#include <WinSock2.h>


inline uint16 HostToNet16(uint32 n) { return htons(n); }

inline uint32 HostToNet32(uint32 n) { return htonl(n); }

inline uint16 NetToHost16(uint16 n) { return ntohs(n); }

inline uint32 NetToHost32(uint32 n) { return ntohl(n); }

#endif // WIN32


#endif  // !#define (PROJECT_BYTE_INCLUDE_H_ )