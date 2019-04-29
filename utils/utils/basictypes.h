///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_BASICTYPES_INCLUDE_H_
#define UTILS_BASICTYPES_INCLUDE_H_

#include <cmath>
#include <functional>

#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#include <WinUser.h>

#include "utils/compiler.h"

#define UTILS_USER_LOWER        WM_USER + 0x07E1            // 07E1->2017, 
#define UTILS_USER_UPPER        WM_USER + 0x7FFF - WM_USER
#define UTILS_APP_LOWER         WM_APP
#define UTILS_APP_UPPER         WM_APP + 0xBFFF - WM_APP
#define UTILS_REG_MESSAGE_LOWER 0xC000
#define UTILS_REG_MESSAGE_UPPER 0xFFFF


#define UTILS_ABSL_COMDAT __declspec(selectany)

// Helper macro which assist user to print enum value with the Literally.
// which use the enum value macro.
#define UTILS_STRINGIZE(x) #x
#define UTILS_LINK(x, y) x##y

typedef signed char         schar;
typedef signed char         int8;
typedef signed int          char32;
typedef short int           int16;
typedef int                 int32;
typedef long long           int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned int        uint;
typedef unsigned char       byte;
typedef unsigned long long  uint64;
typedef wchar_t             char16;

const uint8  kuint8max  = ((uint8)0xFF);
const uint16 kuint16max = ((uint16)0xFFFF);
const uint32 kuint32max = ((uint32)0xFFFFFFFF);
const uint64 kuint64max = ((uint64)GG_LONGLONG(0xFFFFFFFFFFFFFFFF));
const  int8  kint8min   = ((int8)0x80);
const  int8  kint8max   = ((int8)0x7F);
const  int16 kint16min  = ((int16)0x8000);
const  int16 kint16max  = ((int16)0x7FFF);
const  int32 kint32min  = ((int32)0x80000000);
const  int32 kint32max  = ((int32)0x7FFFFFFF);
const  int64 kint64min  = ((int64)GG_LONGLONG(0x8000000000000000));
const  int64 kint64max  = ((int64)GG_LONGLONG(0x7FFFFFFFFFFFFFFF));

// Hack to convert any char-like type to its unsigned counterpart.
// For example, it will convert char, signed char and unsigned char to unsigned
// char.
template<typename T>
struct ToUnsigned {
    typedef T Unsigned;
};

template<>
struct ToUnsigned<char> {
    typedef unsigned char Unsigned;
};
template<>
struct ToUnsigned<signed char> {
    typedef unsigned char Unsigned;
};
template<>
struct ToUnsigned<wchar_t> {
#if defined(WCHAR_T_IS_UTF16)
    typedef unsigned short Unsigned;
#elif defined(WCHAR_T_IS_UTF32)
    typedef uint32 Unsigned;
#endif
};
template<>
struct ToUnsigned<short> {
    typedef unsigned short Unsigned;
};

#ifdef OS_POSIX
#error "without format_macros defined."
#else // OS_WIN

#if !defined(PRId64)
#define PRId64 "I64d"
#endif

#if !defined(PRIu64)
#define PRIu64 "I64u"
#endif

#if !defined(PRIx64)
#define PRIx64 "I64x"
#endif

#define WidePRId64 L"I64d"
#define WidePRIu64 L"I64u"
#define WidePRIx64 L"I64x"

#if !defined(PRIuS)
#define PRIuS "Iu"
#endif

#if !defined(PRDWROD)
#define PRDWROD "lu"
#endif

#if !defined(PRHRESULT)
#define PRHRESULT "08X"
#endif

#endif // OS_WIN

#define WHITESPACE_UNICODE \
  0x0009, /* <control-0009> to <control-000D> */ \
  0x000A,                                        \
  0x000B,                                        \
  0x000C,                                        \
  0x000D,                                        \
  0x0020, /* Space */                            \
  0x0085, /* <control-0085> */                   \
  0x00A0, /* No-Break Space */                   \
  0x1680, /* Ogham Space Mark */                 \
  0x180E, /* Mongolian Vowel Separator */        \
  0x2000, /* En Quad to Hair Space */            \
  0x2001,                                        \
  0x2002,                                        \
  0x2003,                                        \
  0x2004,                                        \
  0x2005,                                        \
  0x2006,                                        \
  0x2007,                                        \
  0x2008,                                        \
  0x2009,                                        \
  0x200A,                                        \
  0x200C, /* Zero Width Non-Joiner */            \
  0x2028, /* Line Separator */                   \
  0x2029, /* Paragraph Separator */              \
  0x202F, /* Narrow No-Break Space */            \
  0x205F, /* Medium Mathematical Space */        \
  0x3000, /* Ideographic Space */                \
  0


// The following enum should be used only as a constructor argument to indicate
// that the variable has static storage class, and that the constructor should
// do nothing to its state.  It indicates to the reader that it is legal to
// declare a static instance of the class, provided the constructor is given
// the utils::LINKER_INITIALIZED argument.  Normally, it is unsafe to declare a
// static variable that has a constructor or a destructor because invocation
// order is undefined.  However, IF the type can be initialized by filling with
// zeroes (which the loader does for static variables), AND the destructor also
// does nothing to the storage, AND there are no virtual methods, then a
// constructor declared as
//       explicit MyClass(utils::LinkerInitialized x) {}
// and invoked as
//       static MyClass my_variable_name(utils::LINKER_INITIALIZED);
namespace utils {

enum LinkerInitialized { LINKER_INITIALIZED };

// Use these to declare and define a static local variable (static T;) so that
// it is leaked so that its destructors are not called at exit.
#define CR_DEFINE_STATIC_LOCAL(type, name, arguments) \
  static type& name = *new type arguments

}  // namespace utils


// Put this in the private: declarations for a class to be uncopyable.
#define DISALLOW_COPY(TypeName) \
  TypeName(const TypeName&)

// Put this in the private: declarations for a class to be unassignable.
#define DISALLOW_ASSIGN(TypeName) \
  void operator=(const TypeName&)

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// An older, deprecated, politically incorrect name for the above.
// NOTE: The usage of this macro was baned from our code base, but some
// third_party libraries are yet using it.
// TODO(tfarina): Figure out how to fix the usage of this macro in the
// third_party libraries and get rid of it.
#ifndef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) DISALLOW_COPY_AND_ASSIGN(TypeName)
#endif

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.

#ifndef DISALLOW_IMPLICIT_CONSTRUCTORS
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                    \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif

// The arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that arraysize() doesn't accept any array of an
// anonymous type or a type defined inside a function.  In these rare
// cases, you have to use the unsafe ARRAYSIZE_UNSAFE() macro below.  This is
// due to a limitation in C++'s template system.  The limitation might
// eventually be removed, but it hasn't happened yet.

// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char(&ArraySizeHelper(T(&array)[N]))[N];

// That gcc wants both of these prototypes seems mysterious. VC, for
// its part, can't decide which to use (another mystery). Matching of
// template overloads: the final frontier.
#ifndef _MSC_VER
template <typename T, size_t N>
char(&ArraySizeHelper(const T(&array)[N]))[N];
#endif

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

// ARRAYSIZE_UNSAFE performs essentially the same calculation as arraysize,
// but can be used on anonymous types or types defined inside
// functions.  It's less safe than arraysize as it accepts some
// (although not all) pointers.  Therefore, you should use arraysize
// whenever possible.
//
// The expression ARRAYSIZE_UNSAFE(a) is a compile-time constant of type
// size_t.
//
// ARRAYSIZE_UNSAFE catches a few type errors.  If you see a compiler error
//
//   "warning: division by zero in ..."
//
// when using ARRAYSIZE_UNSAFE, you are (wrongfully) giving it a pointer.
// You should only use ARRAYSIZE_UNSAFE on statically allocated arrays.
//
// The following comments are on the implementation details, and can
// be ignored by the users.
//
// ARRAYSIZE_UNSAFE(arr) works by inspecting sizeof(arr) (the # of bytes in
// the array) and sizeof(*(arr)) (the # of bytes in one array
// element).  If the former is divisible by the latter, perhaps arr is
// indeed an array, in which case the division result is the # of
// elements in the array.  Otherwise, arr cannot possibly be an array,
// and we generate a compiler error to prevent the code from
// compiling.
//
// Since the size of bool is implementation-defined, we need to cast
// !(sizeof(a) & sizeof(*(a))) to size_t in order to ensure the final
// result has type size_t.
//
// This macro is not perfect as it wrongfully accepts certain
// pointers, namely where the pointer size is divisible by the pointee
// size.  Since all our code has to go through a 32-bit compiler,
// where a pointer is 4 bytes, this means all pointers to a type whose
// size is 3 or greater than 4 will be (righteously) rejected.

#define ARRAYSIZE_UNSAFE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

// The COMPILE_ASSERT macro can be used to verify that a compile time
// expression is true. For example, you could use it to verify the
// size of a static array:
//
//   COMPILE_ASSERT(ARRAYSIZE_UNSAFE(content_type_names) == CONTENT_NUM_TYPES,
//                  content_type_names_incorrect_size);
//
// or to make sure a struct is smaller than a certain size:
//
//   COMPILE_ASSERT(sizeof(foo) < 128, foo_too_large);
//
// The second argument to the macro is the name of the variable. If
// the expression is false, most compilers will issue a warning/error
// containing the name of the variable.

template <bool>
struct CompileAssert {
};

#undef COMPILE_ASSERT
#define COMPILE_ASSERT(expr, msg) \
  typedef CompileAssert<(bool(expr))> msg[bool(expr) ? 1 : -1]

// Implementation details of COMPILE_ASSERT:
//
// - COMPILE_ASSERT works by defining an array type that has -1
//   elements (and thus is invalid) when the expression is false.
//
// - The simpler definition
//
//     #define COMPILE_ASSERT(expr, msg) typedef char msg[(expr) ? 1 : -1]
//
//   does not work, as gcc supports variable-length arrays whose sizes
//   are determined at run-time (this is gcc's extension and not part
//   of the C++ standard).  As a result, gcc fails to reject the
//   following code with the simple definition:
//
//     int foo;
//     COMPILE_ASSERT(foo, msg); // not supposed to compile as foo is
//                               // not a compile-time constant.
//
// - By using the type CompileAssert<(bool(expr))>, we ensures that
//   expr is a compile-time constant.  (Template arguments must be
//   determined at compile-time.)
//
// - The outer parentheses in CompileAssert<(bool(expr))> are necessary
//   to work around a bug in gcc 3.4.4 and 4.0.1.  If we had written
//
//     CompileAssert<bool(expr)>
//
//   instead, these compilers will refuse to compile
//
//     COMPILE_ASSERT(5 > 0, some_message);
//
//   (They seem to think the ">" in "5 > 0" marks the end of the
//   template argument list.)
//
// - The array size is (bool(expr) ? 1 : -1), instead of simply
//
//     ((expr) ? 1 : -1).
//
//   This is to avoid running into a bug in MS VC 7.1, which
//   causes ((0.0) ? 1 : -1) to incorrectly evaluate to 1.



// Returns the integer i such as 2^i <= n < 2^(i+1)
inline int Log2Floor(uint32 n) {
    if (n == 0) return -1;
    int log = 0;
    uint32 value = n;
    for (int i = 4; i >= 0; --i) {
        int shift = (1 << i);
        uint32 x = value >> shift;
        if (x != 0) {
            value = x;
            log += shift;
        }
    }
    return log;
}

// Returns the integer i such as 2^(i-1) < n <= 2^i
inline int Log2Ceiling(uint32 n) {
    if (n == 0) {
        return -1;
    } else {
        // Log2Floor returns -1 for 0, so the following works correctly for n=1.
        return 1 + Log2Floor(n - 1);
    }
}

// float_util
inline bool IsFinite(const double& number) {
#if defined(OS_POSIX)
    return std::isfinite(number) != 0;
#elif defined(OS_WIN)
    return _finite(number) != 0;
#endif
}

#endif  // !UTILS_BASICTYPES_INCLUDE_H_