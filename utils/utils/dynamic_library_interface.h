///////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_DYNAMIC_LIBRARY_INTERFACE_INCLUDE_H_
#define UTILS_DYNAMIC_LIBRARY_INTERFACE_INCLUDE_H_

#include <mutex>

#include "utils/dynamic_library.h"

namespace utils {

namespace subtle {

template<typename NativeInterface>
struct Pointer {
static void Destruct(const std::weak_ptr<DynamicLibrary>& library, const std::string& InterfaceName, NativeInterface** inter) {
    if (!inter) return;
    NativeInterface* ptr = *inter;
    *inter = nullptr;
    auto destroctor = GetFunctionPointer<void, NativeInterface*>(library, InterfaceName);
    if (!destroctor) return;
    destroctor(ptr);
}
};

template<typename NativeInterface>
struct DoublePointer {
static void Destruct(const std::weak_ptr<DynamicLibrary>& library, const std::string& InterfaceName, NativeInterface** inter) {
    if (!inter) return;
    NativeInterface* ptr = *inter;
    *inter = nullptr;
    auto destroctor = GetFunctionPointer<void, NativeInterface**>(library, InterfaceName);
    if (!destroctor) return;
    destroctor(&ptr);
}
};

template<typename NativeInterface, typename DestructTraits>
class NativeTraits {
public:
    using Destructor = std::function<void(NativeInterface**)>;

    explicit NativeTraits(NativeInterface* inter, const Destructor& destructor)
        : inteface_(inter)
        , destructor_(destructor) {}

    explicit NativeTraits(const std::weak_ptr<DynamicLibrary>& library, const std::string& CreateInterface, const std::string& DestroyInterface)
        : interface_(NativeTraits::Contruct(library, CreateInterface))
        , destructor_(NativeTraits::Destruct(library, DestroyInterface)) {}

    template<typename... P>
    explicit NativeTraits(const std::weak_ptr<DynamicLibrary>& library, const std::string& CreateInterface, const std::string& DestroyInterface, P&&... args)
        : interface_(NativeTraits::Contruct<P...>(library, CreateInterface, std::forward<P...>(args)...))
        , destructor_(NativeTraits::Destruct(library, DestroyInterface)) {}

    virtual ~NativeTraits() {
        if (!interface_ || !destructor_) return;
        destructor_(&interface_);
    }

    template<typename... P>
    static NativeInterface* Contruct(const std::weak_ptr<DynamicLibrary>& library, const std::string& InterfaceName, P... args) {
#ifdef TEST
        const unsigned number = sizeof...(P);
#endif
        auto contructor = GetFunctionPointer<NativeInterface*, P...>(library, InterfaceName);
        if (!contructor) return nullptr;
        return contructor(args...);
    }

    static Destructor Destruct(const std::weak_ptr<DynamicLibrary>& library, const std::string& InterfaceName) {
        return [library, InterfaceName](NativeInterface** inter) {
            DestructTraits::Destruct(library, InterfaceName, inter);
        };
    }

    NativeInterface* get() const { return interface_; }

protected:
    Destructor destructor_;
    NativeInterface* interface_ = nullptr;
};

template<typename NativeInterface>
using PointerTraits = NativeTraits<NativeInterface, Pointer<NativeInterface>>;

template<typename NativeInterface>
using DoublePointerTraits = NativeTraits<NativeInterface, DoublePointer<NativeInterface>>;

class ThreadFlag {
public:
    ThreadFlag() : valid_thread_id_(::GetCurrentThreadId()) {}
    virtual ~ThreadFlag() {}

    bool CalledOnValidThread() const {
        std::lock_guard<std::mutex> guard(lock_);
        return valid_thread_id_ == ::GetCurrentThreadId();
    }

private:
    mutable std::mutex lock_;
    mutable DWORD valid_thread_id_ = 0;
};

} // namespace subtle


template<typename NativeInterface, typename Traits = subtle::PointerTraits<NativeInterface>>
class Interface {
public:
    explicit Interface() {}
    explicit Interface(const Interface& r) { *this = r; }
    virtual ~Interface() { reset(); }
    Interface& operator=(const Interface& r) {
        interface_ = r.interface_;
        weak_interface_ = r.weak_interface_;
        library_name_ = r.library_name_;
        if (!interface_) weak_flag_ = r.weak_flag_;
        return *this;
    }

    Interface AstWeakPtr() {
        Interface tmp;
        tmp.library_name_ = library_name_;
        if (interface_) tmp.weak_interface_ = interface_;
        else tmp.weak_interface_ = weak_interface_;
        if (!flag_) flag_ = new subtle::ThreadFlag();
        tmp.weak_flag_ = flag_;
        return tmp;
    }

    Interface AsRefPtr() {
        Interface tmp;
        tmp.library_name_ = library_name_;
        if (interface_) tmp.interface_ = interface_;
        else tmp.interface_ = weak_interface_.lock();
        return tmp;
    }

public:
    template<typename... P>
    explicit Interface(const std::shared_ptr<DynamicLibrary>& library, const std::string& CreateInterface, const std::string& DestroyInterface, P... args)
        : interface_(new Traits(library, CreateInterface, DestroyInterface, std::forward<P...>(args)...))
        , library_name_(DynamicLibrary::GetLibraryName(library)){}

    void Reset(const std::shared_ptr<DynamicLibrary>& library, const std::string& CreateInterface, const std::string& DestroyInterface) {
        interface_ = new Traits(library, CreateInterface, DestroyInterface);
        library_name_ = DynamicLibrary::GetLibraryName(library);
    }

    template<typename... P>
    void Reset(const std::shared_ptr<DynamicLibrary>& library, const std::string& CreateInterface, const std::string& DestroyInterface, P... args) {
        interface_ = new Traits(library, CreateInterface, DestroyInterface, std::forward<P...>(args)...);
        library_name_ = DynamicLibrary::GetLibraryName(library);
    }

    void Reset(NativeInterface* inter, const typename Traits::Destructor& destructor) { interface_ = new Traits(inter, destructor); }

public:
    operator bool() const { return !!get(); }

    bool operator!() const { return !get(); }

    Interface& operator=(std::nullptr_t) {
        reset();
        return *this;
    }

    NativeInterface* get() const {
      if (!library_name_.empty() && !utils::WellKnownLibrary(library_name_))
        return nullptr;
        if (interface_) return interface_->get();
        auto known_interface = weak_interface_.lock();
        if (known_interface) {
            auto flag = weak_flag_.lock();
            if (flag) return known_interface->get();
        }
        return nullptr;
    }

    operator NativeInterface*() const {
        auto _interface = get();
        assert(_interface != nullptr);
        return _interface;
    }

    NativeInterface* operator->() const {
        auto _interface = get();
        assert(_interface != nullptr);
        return _interface;
    }

    void reset() { interface_ = nullptr; weak_interface_ = nullptr; library_name_ = L""; }

    void swap(Interface& r) { interface_.swap(r.interface_); weak_interface_.swap(r.weak_interface_); std::swap(libarary_name_, r.library_name_); }

    void SetLibraryName(const std::wstring& name) { library_name_ = name; }

protected:
    std::shared_ptr<Traits> interface_ = nullptr;
    std::weak_ptr<Traits> weak_interface_;
    std::wstring library_name_;
    std::shared_ptr<subtle::ThreadFlag> flag_;
    std::weak_ptr<subtle::ThreadFlag> weak_flag_;
};

template<typename R, typename... P>
class Function {
public:
    explicit Function() {}
    explicit Function(std::string name) { Reset(nullptr, name); }
    explicit Function(const std::shared_ptr<DynamicLibrary>& library, std::string name) {
        Reset(library, name);
    }
    virtual ~Function() { reset(); }

    void Reset(const std::string& name) { Reset(library_, name); }

    void Reset(const std::shared_ptr<DynamicLibrary>& library) { Reset(library, name_); }

    void Reset(const std::shared_ptr<DynamicLibrary>& library, const std::string& name) {
        library_ = library;
        name_ = name;
        if (!library || name.empty()) return;
        function_ = utils::GetFunctionPointer<R, P...>(library, name);
        if (!function_) library_ = nullptr;
    }

    Function& operator=(std::nullptr_t) {
        reset();
        return *this;
    }

    operator bool() const { return !!get(); }

    bool operator!() const { return !get(); }

    R operator()(P&&... args) {
        assert(function_ != nullptr && library_ != nullptr);
        return function_(std::forward<P>(args)...);
    }

    void reset() { library_ = nullptr; function_ = nullptr; name_ = ""; }

    void swap(Function& r) { library_.swap(r.library_); weak_library_.swap(r.weak_library_); std::swap(name_, r.name_); std::swap(function_, r.function_); }

protected:
    typename FunctorTraits<R, P...>::Type get() const {
        if (library_) return function_;
        return nullptr;
    }

    std::shared_ptr<DynamicLibrary> library_;
    std::string name_;
    typename FunctorTraits<R, P...>::Type function_ = nullptr;
};

} // namespace utils

#endif  // !UTILS_DYNAMIC_LIBRARY_INTERFACE_INCLUDE_H_