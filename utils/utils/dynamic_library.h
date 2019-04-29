///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_DYNAMIC_LIBRARY_INCLUDE_H_
#define UTILS_DYNAMIC_LIBRARY_INCLUDE_H_

#include <memory>
#include <Windows.h>

#include "utils.h"
#include "utils/files/file_util.h"

namespace utils {

template<typename R, typename... P>
struct FunctorTraits { using Type = R(WINAPI *)(P...); };

class UTILS_API DynamicLibrary {
 public:
    explicit DynamicLibrary() {}
    explicit DynamicLibrary(const HMODULE& library) : library_(library) {}
    explicit DynamicLibrary(const std::wstring& path)
        : library_(utils::LoadLibrary(path, nullptr)) {}
    explicit DynamicLibrary(LPCTSTR filename) : library_name_(filename) {}
    virtual ~DynamicLibrary() { UnloadNativeLibrary(library_); }

    bool is_valid() const { return !!library_ || WellKnownLibrary(library_name_); }

    // Only validate when this object hold the wellknown library's handler.
    const std::wstring& library_name() const { return library_name_; }

    void* GetFunctionPointer(const char* function_name) const {
        if (!is_valid()) return nullptr;
        if (library_name_.empty()) return GetFunctionPointerFromNativeLibrary(library_, function_name);
        return GetFunctionPointerFromNativeLibrary(library_name_, function_name);
    }

    template<typename R, typename... P>
    typename FunctorTraits<R, P...>::Type GetFunctionPointer(const std::string& InterfaceName) const {
        if (!is_valid() && InterfaceName.empty()) return nullptr;
        using Type = FunctorTraits<R, P...>::Type;
        return reinterpret_cast<Type>(DynamicLibrary::GetFunctionPointer(InterfaceName.c_str()));
    }

    void Reset(HMODULE library) {
        UnloadNativeLibrary(library_);
        library_ = library;
    }

    // Returns the native library handle and removes it from this object. The
    // caller must manage the lifetime of the handle.
    // Otherwise, when we hold the wellknown library's handler, result should be nullptr.
    auto Release() {
        auto result = library_;
        library_ = nullptr;
        return result;
    }

private:
    std::wstring library_name_;
    HMODULE library_ = nullptr;
    DISALLOW_COPY_AND_ASSIGN(DynamicLibrary);
};

template<typename R, typename... P>
typename FunctorTraits<R, P...>::Type GetFunctionPointer(const HMODULE& library, const std::string& InterfaceName) {
    if (!library || InterfaceName.empty()) return nullptr;
    using Type = FunctorTraits<R, P...>::Type;
    return reinterpret_cast<Type>(GetFunctionPointerFromNativeLibrary(library, InterfaceName.c_str()));
}

template<typename R, typename... P>
typename FunctorTraits<R, P...>::Type GetFunctionPointer(const DynamicLibrary* library, const std::string& InterfaceName) {
    if (!library) return nullptr;
    return library->GetFunctionPointer<R, P...>(InterfaceName.c_str());
}

template<typename R, typename... P>
typename FunctorTraits<R, P...>::Type GetFunctionPointer(const std::weak_ptr<DynamicLibrary>& library, const std::string& InterfaceName) {
    auto known_library = library.lock();
    if (!known_library) return nullptr;
    return known_library->GetFunctionPointer<R, P...>(InterfaceName.c_str());
}

} // namespace utils

#endif  // !UTILS_DYNAMIC_LIBRARY_INCLUDE_H_