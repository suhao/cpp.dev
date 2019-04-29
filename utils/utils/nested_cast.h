///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_HIERARCHY_CAST_INCLUDE_H_
#define UTILS_HIERARCHY_CAST_INCLUDE_H_

#include <assert.h>    // for assert
#include <string.h>    // for memcpy
#include <functional>  // for std::function
#include <string>

//
template <typename T>
std::basic_string<T> safety_string(const T* elem) {
  if (nullptr == elem) return std::basic_string<T>();
  return elem;
}

template <typename T, size_t size>
std::basic_string<T> safety_array_string(const T (&elem)[size]) {
  return std::basic_string<T>(elem, size);
}

// https://marknelson.us/posts/2011/09/03/hash-functions-for-c-unordered-containers.html
template <typename T>
std::size_t nested_cast(T* t) {
  return reinterpret_cast<std::size_t>(t);
}

// Used to explicitly mark the return value of a function as unused. If you are
// really sure you don't want to do anything with the return value of a function
// that has been marked WARN_UNUSED_RESULT, wrap it with this. Example:
//
//   scoped_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     ignore_result(my_var.release());
//
template <typename T>
inline void ignore_result(const T&) {}

template <typename... ARGS>
inline void nothing(ARGS... args) {}

template <typename... ARGS>
inline void evaluate(const std::function<void(ARGS...)>& func, ARGS... args) {
  if (!func) {
    return;
  }
  return func(std::forward<Params>(args)...);
}

template <typename R, typename... ARGS>
inline R evaluate(const std::function<R(ARGS...)>& func, ARGS... args) {
  if (!func) {
    return R();
  }
  return func(std::forward<Params>(args)...);
}

template <typename R, typename... ARGS>
struct simulate {
  using Simulate = std::function<R(ARGS&&...)>;

  simulate(R tacitly, const Simulate& func)
      : tacitly_(tacitly), simulate_(func) {}

  R operator()(ARGS&&... args) {
    assert(!!simulate_);
    return simulate_(std::forward<P>(args)...);
  }

private:
  Simulate simulate_;
  R tacitly_;
};

// bit_cast<Dest,Source> is a template function that implements the
// equivalent of "*reinterpret_cast<Dest*>(&source)".  We need this in
// very low-level functions like the protobuf library and fast math
// support.
//
//   float f = 3.14159265358979;
//   int i = bit_cast<int32>(f);
//   // i = 0x40490fdb
//
// The classical address-casting method is:
//
//   // WRONG
//   float f = 3.14159265358979;            // WRONG
//   int i = * reinterpret_cast<int*>(&f);  // WRONG
//
// The address-casting method actually produces undefined behavior
// according to ISO C++ specification section 3.10 -15 -.  Roughly, this
// section says: if an object in memory has one type, and a program
// accesses it with a different type, then the result is undefined
// behavior for most values of "different type".
//
// This is true for any cast syntax, either *(int*)&f or
// *reinterpret_cast<int*>(&f).  And it is particularly true for
// conversions betweeen integral lvalues and floating-point lvalues.
//
// The purpose of 3.10 -15- is to allow optimizing compilers to assume
// that expressions with different types refer to different memory.  gcc
// 4.0.1 has an optimizer that takes advantage of this.  So a
// non-conforming program quietly produces wildly incorrect output.
//
// The problem is not the use of reinterpret_cast.  The problem is type
// punning: holding an object in memory of one type and reading its bits
// back using a different type.
//
// The C++ standard is more subtle and complex than this, but that
// is the basic idea.
//
// Anyways ...
//
// bit_cast<> calls memcpy() which is blessed by the standard,
// especially by the example in section 3.9 .  Also, of course,
// bit_cast<> wraps up the nasty logic in one place.
//
// Fortunately memcpy() is very fast.  In optimized mode, with a
// constant size, gcc 2.95.3, gcc 4.0.1, and msvc 7.1 produce inline
// code with the minimal amount of data movement.  On a 32-bit system,
// memcpy(d,s,4) compiles to one load and one store, and memcpy(d,s,8)
// compiles to two loads and two stores.
//
// I tested this code with gcc 2.95.3, gcc 4.0.1, icc 8.1, and msvc 7.1.
//
// WARNING: if Dest or Source is a non-POD type, the result of the memcpy
// is likely to surprise you.

template <class Dest, class Source>
inline Dest bit_cast(const Source& source) {
  // Compile time assertion: sizeof(Dest) == sizeof(Source)
  // A compile error here means your Dest and Source have different sizes.
  typedef char VerifySizesAreEqual[sizeof(Dest) == sizeof(Source) ? 1 : -1];

  Dest dest;
  memcpy(&dest, &source, sizeof(dest));
  return dest;
}

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
//
// When you upcasting (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasting
// always succeed.
template <typename To, typename From>
inline To implicit_cast(From const& f) {
  return f;
}

// When you downcasting (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?
//
// It could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus,
// when you downcasting, you should use this macro.
//
// In debug mode, we use dynamic_cast<> to double-check the downcast
// is legal (we die // if it's not).  In normal mode, we do the
// efficient static_cast<> instead.
//
// Ensures that To is a sub-type of From *.  This test is here only
// for compile-time type checking, and has no overhead in an
// optimized build at run-time, as it will be optimized away
// completely.
//    if (false) implicit_cast<From*, To>(0);
//
// @Ö£ÖØ¾¯¸æ:
//    http://baiy.cn/doc/cpp/inside_rtti.htm
//    https://www.zhihu.com/question/22445339
//
// dynamic_cast maybe throw a exception: std::bad_cast
//    dynamic_cast<Foo&>(expr)->throw std::bad_cast
//    dynamic_cast<Foo*>(expr)->nullptr or pointer
//
// Thus, it's important to test in debug mode to make sure
// the cast is legal!
//
// This is the only place in the code we should use dynamic_cast<>.
//
// In particular, you SHOULDN'T be using dynamic_cast<> in order to
// do RTTI (eg code like this:
//    if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
//    if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
// You should design the code some other way not to need this.
//
// @Solemn Warning << std::end;

template <typename To, typename From>  // use like this: super_cast<T*>(foo);
inline To super_cast(From* f) {        // so we only accept pointers
  if (false) {
    implicit_cast<From*, To>(0);
  }
#if !defined(NDEBUG) && !defined(X_NO_RTTI)
  assert(f == nullptr ||
         dynamic_cast<To>(f) != nullptr);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

// Typecasting between |function| and |PVOID|:
//    typedef void(Foo::*MemberFunc)();
//    auto address = &Foo::func;
//    auto pointer = function_cast<void*>(&Foo::func);
//    auto static_pointer = *static_cast<T*>(static_cast<void*>(&(&Foo::func)));
//    auto func = function_cast<MemberFunc>(pointer);
//
//    typedef void(*GlobalFunc)();
//    auto address = &function;
//    auto reinterpret_pointer = reinterpret_cast<void*>(&function);
//    auto static_pointer = static_cast<void*>(&function);
//    auto function_pointer = function_cast<void*>(&function);
//
//    auto global_func_addr = reinterpret_cast<GlobalFunc>(&function);
//    auto global_func_reinterpret_addr = reinterpret_cast<GlobalFunc>(address);
//    auto global_func_static_addr = static_cast<GlobalFunc>(address);
//    auto global_func_function_addr = function_cast<GlobalFunc>(address);
template <typename T, typename P>
T function_cast(P p) {
  return *reinterpret_cast<T*>(static_cast<void*>(&p));
}

// nested_cast->objects:
// we could use static_cast or reinterpret_cast.
//    static_cast<void*>(T*)
//    static_cast<T*>(void*)
//    reinterpret_cast<void*>(T*)
//    reinterpret_cast<T*>(void*)
//    reinterpret_cast<Parent*>(dynamic_cast<Parent*>(Child*))
//    dynamic_cast<void*>(T*)
//    dynamic_cast<Parent*>(dynamic_cast<Parent*>(Child*)) =
//    dynamic_cast<void*>(Child*)
//
template <typename U, typename V>
void* nested_cast(V* v) {
  return static_cast<void*>(dynamic_cast<U*>(v));
}
template <typename Parent, typename Child>
void* nested_implicit_cast(Child* child) {
  return static_cast<void*>(implicit_cast<Parent*>(child));
}
template <typename Parent, typename Child>
void* nested_super_cast(Parent* parent) {
  return static_cast<void*>(super_cast<Child*>(parent));
}

/*****************************************************************************************
// class Child : public Parent1, public Parent2
//    void* parent1 = nested_static_cast(child);
//    void* parent2 = nested_static_cast(child);

template<typename T>
class nested_static_cast {
public:
    template<typename U> nested_static_cast(U* u) : ptr_(dynamic_cast<T*>(u)) {}
    operator T*() const { return ptr_; }                        //
dynamic_cast<T*>(u) operator void*() const { return nested_static_cast(ptr_); }
// static_cast<void*>(u) || nested_cast

private:
    T * ptr_ = nullptr;
};
*****************************************************************************************/
template <typename T>
void* nested_static_cast(T* t) {
  return static_cast<void*>(t);
}
template <typename T>
T* nested_static_cast(void* t) {
  return static_cast<T*>(t);
}

template <typename T>
void* nested_reinterpret_cast(T* t) {
  return reinterpret_cast<void*>(t);
}
template <typename T>
T* nested_reinterpret_cast(void* t) {
  return reinterpret_cast<T*>(t);
}

/*****************************************************************************************

template<typename T>
class nested_dynamic_cast {
public:
    template<typename U> nested_dynamic_cast(U* u) : ptr_(dynamic_cast<T*>(u))
{} operator T*() const { return ptr_; }                        //
dynamic_cast<T*>(u) operator void*() const { return dynamic_cast<void*>(ptr_); }
// dynamic_cast<void*>(u) private: T * ptr_ = nullptr;
};

*****************************************************************************************/
template <typename T>
void* nested_dynamic_cast(T* t) {
  return dynamic_cast<void*>(t);
}
template <typename U, typename V>
void* nested_dynamic_cast(V* v) {
  return nested_dynamic_cast(dynamic_cast<U*>(v));
}

template <typename U, typename V>
bool nested_equal(const U* u, const V* v) {
  return nested_dynamic_cast(u) == nested_dynamic_cast(v);
}

template <typename T>
bool nested_equal(const T* u, const T* v) {
  return nested_dynamic_cast(u) == nested_dynamic_cast(v);
}

#endif  // !#define (UTILS_HIERARCHY_CAST_INCLUDE_H_ )
