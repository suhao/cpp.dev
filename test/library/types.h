/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef TEST_TYPES_INCLUDE_H_ 
#define TEST_TYPES_INCLUDE_H_ 

#include <string>

#ifdef COMPILER_MSVC
typedef __int64 int64;
#else
typedef long long int64;
#endif /* COMPILER_MSVC */

typedef signed char         schar;
typedef signed char         int8;
typedef short               int16;
typedef int                 int32;

#ifdef COMPILER_MSVC
typedef unsigned __int64 uint64;
typedef __int64 int64;
#else
typedef unsigned long long uint64;
typedef long long int64;
#endif /* COMPILER_MSVC */

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned char      byte;
typedef unsigned int       uint;


#ifdef WIN32
typedef int socklen_t;
#endif

template<typename T>
inline T _max(T a, T b) { return a > b ? a : b; }


template<typename T>
inline T _min(T a, T b) { return a > b ? b : a; }


#ifndef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
	TypeName(const TypeName&);                  \
	void operator=(const TypeName&)

#endif

#ifndef DISALLOW_IMPLICIT_CONSTRUCTORS
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
	TypeName();                                    \
	DISALLOW_EVIL_CONSTRUCTORS(TypeName)

#endif

#endif  // !#define (TEST_TYPES_INCLUDE_H_ )