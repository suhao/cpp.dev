/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef UTILS_COMPILER_INCLUDE_H_ 
#define UTILS_COMPILER_INCLUDE_H_ 

// A set of macros to use for platform detection.
#if defined(_WIN32)
#define OS_WIN 1
#define TOOLKIT_VIEWS 1
#else
#error Please add support for your platform
#endif


/*
 * MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
 * MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
 * MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
 * MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
 * MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
 * MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
 * MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
 * MSVC++ 7.0  _MSC_VER == 1300
 * MSVC++ 6.0  _MSC_VER == 1200
 * MSVC++ 5.0  _MSC_VER == 1100
 */

// Compiler detection.
#if defined(__GNUC__)
#define COMPILER_GCC 1
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#else
#error Please add support for your compiler in build/config.h
#endif

#ifdef COMPILER_MSVC
#define GG_LONGLONG(x) x##I64
#define GG_ULONGLONG(x) x##UI64
#else
#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
#endif

// MSVC_SUPPRESS_WARNING disables warning |n| for the remainder of the line and
// for the next line of the source file.
#define MSVC_SUPPRESS_WARNING(n) __pragma(warning(suppress:n))

// MSVC_PUSH_DISABLE_WARNING pushes |n| onto a stack of warnings to be disabled.
// The warning remains disabled until popped by MSVC_POP_WARNING.
#define MSVC_PUSH_DISABLE_WARNING(n) __pragma(warning(push)) \
                                     __pragma(warning(disable:n))

// MSVC_PUSH_WARNING_LEVEL pushes |n| as the global warning level.  The level
// remains in effect until popped by MSVC_POP_WARNING().  Use 0 to disable all
// warnings.
#define MSVC_PUSH_WARNING_LEVEL(n) __pragma(warning(push, n))

// Pop effects of innermost MSVC_PUSH_* macro.
#define MSVC_POP_WARNING() __pragma(warning(pop))

#define MSVC_DISABLE_OPTIMIZE() __pragma(optimize("", off))
#define MSVC_ENABLE_OPTIMIZE() __pragma(optimize("", on))

// Allows exporting a class that inherits from a non-exported base class.
// This uses suppress instead of push/pop because the delimiter after the
// declaration (either "," or "{") has to be placed before the pop macro.
//
// Example usage:
// class EXPORT_API Foo : NON_EXPORTED_BASE(public Bar) {
//
// MSVC Compiler warning C4275:
// non dll-interface class 'Bar' used as base for dll-interface class 'Foo'.
// Note that this is intended to be used only when no access to the base class'
// static data is done through derived classes or inline methods. For more info,
// see http://msdn.microsoft.com/en-us/library/3tdb471s(VS.80).aspx
#define NON_EXPORTED_BASE(code) MSVC_SUPPRESS_WARNING(4275) \
                                code

// Annotate a variable indicating it's ok if the variable is not used.
// (Typically used to silence a compiler warning when the assignment
// is important for some other reason.)
// Use like:
//   int x ALLOW_UNUSED = ...;
#if defined(COMPILER_GCC)
#define ALLOW_UNUSED __attribute__((unused))
#else
#define ALLOW_UNUSED
#endif

// Annotate a function indicating it should not be inlined.
// Use like:
//   NOINLINE void DoStuff() { ... }
#if defined(COMPILER_GCC)
#define NOINLINE __attribute__((noinline))
#elif defined(COMPILER_MSVC)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

// Specify memory alignment for structs, classes, etc.
// Use like:
//   class ALIGNAS(16) MyClass { ... }
//   ALIGNAS(16) int array[4];
#if defined(COMPILER_MSVC)
#define ALIGNAS(byte_alignment) __declspec(align(byte_alignment))
#elif defined(COMPILER_GCC)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#endif

// Return the byte alignment of the given type (available at compile time).  Use
// sizeof(type) prior to checking __alignof to workaround Visual C++ bug:
// http://goo.gl/isH0C
// Use like:
//   ALIGNOF(int32)  // this would be 4
#if defined(COMPILER_MSVC)
#define ALIGNOF(type) (sizeof(type) - sizeof(type) + __alignof(type))
#elif defined(COMPILER_GCC)
#define ALIGNOF(type) __alignof__(type)
#endif

// Annotate a virtual method indicating it must be overriding a virtual
// method in the parent class.
// Use like:
//   virtual void foo() OVERRIDE;
#if defined(COMPILER_MSVC)
#define OVERRIDE override
#elif defined(__clang__)
#define OVERRIDE override
#else
#define OVERRIDE
#endif

// Annotate a virtual method indicating that subclasses must not override it,
// or annotate a class to indicate that it cannot be subclassed.
// Use like:
//   virtual void foo() FINAL;
//   class B FINAL : public A {};
#if defined(COMPILER_MSVC)
// TODO(jered): Change this to "final" when chromium no longer uses MSVC 2010.
#define FINAL sealed
#elif defined(__clang__)
#define FINAL final
#else
#define FINAL
#endif

// Annotate a function indicating the caller must examine the return value.
// Use like:
//   int foo() WARN_UNUSED_RESULT;
// To explicitly ignore a result, see |ignore_result()| in <base/basictypes.h>.
#if defined(COMPILER_GCC)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

// Tell the compiler a function is using a printf-style format string.
// |format_param| is the one-based index of the format string parameter;
// |dots_param| is the one-based index of the "..." parameter.
// For v*printf functions (which take a va_list), pass 0 for dots_param.
// (This is undocumented but matches what the system C headers do.)
#if defined(COMPILER_GCC)
#define PRINTF_FORMAT(format_param, dots_param) \
    __attribute__((format(printf, format_param, dots_param)))
#else
#define PRINTF_FORMAT(format_param, dots_param)
#endif

// WPRINTF_FORMAT is the same, but for wide format strings.
// This doesn't appear to yet be implemented in any compiler.
// See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=38308 .
#define WPRINTF_FORMAT(format_param, dots_param)
// If available, it would look like:
//   __attribute__((format(wprintf, format_param, dots_param)))


// Disable: 4251 4275
#if defined(COMPILER_MSVC)
#pragma warning(disable:4251 4275)
#elif
#endif

#endif  // !#define (UTILS_COMPILER_INCLUDE_H_ )