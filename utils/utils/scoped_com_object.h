/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_COM_OBJECT_INCLUDE_H_ 
#define DIRECTX_SCOPED_COM_OBJECT_INCLUDE_H_ 

#include <unknwn.h>
#include <assert.h>

#include "scoped_ref_object.h"

template<class Interface, bool RefCounted = false, const IID* id = &__uuidof(Interface)>
class ScopedComObject : public scoped_refptr<Interface, RefCounted> {
public:
    using Parent = scoped_refptr<Interface, RefCounted>;
    class IUnknownMethods : public Interface {
    private:
        STDMETHOD(QueryInterface)(REFIID iid, void** object) = 0;
        STDMETHOD_(ULONG, AddRef)() = 0;
        STDMETHOD_(ULONG, Release)() = 0;
    };

    ScopedComObject() {}
    ScopedComObject(std::nullptr_t) : Parent(nullptr) {}

    explicit ScopedComObject(Interface* p) : Parent(p) {}

    ScopedComObject(const ScopedComObject<Interface>& p)
        : Parent(p) {}

    virtual ~ScopedComObject() {}

    ScopedComObject& operator=(std::nullptr_t) {
        Release();
        return *this;
    }

    ScopedComObject& operator=(Interface* p) {
        if (p != ptr_) {
            ScopedComObject(p).swap(*this);
        }
        return *this;
    }

    template <typename U>
    ScopedComObject& operator=(U *p) {
        ComPtr(p).swap(*this);
        return *this;
    }

    ScopedComObject& operator=(const ScopedComObject &p) {
        if (ptr_ != p.ptr_) { ScopedComObject(p).swap(*this); }
        return *this;
    }

    template<class U>
    ScopedComObject& operator=(const ScopedComObject<U>& p) {
        ScopedComObject(other).swap(*this);
        return *this;
    }

    void Release() {
        if (ptr_ != NULL) {
            ptr_->Release();
            ptr_ = NULL;
        }
    }

    void Attach(Interface* p) {
        assert(!ptr_);
        ptr_ = p;
    }

    Interface* Detach() {
        Interface* p = ptr_;
        ptr_ = NULL;
        return p;
    }

    Interface** Receive() {
        assert(!ptr_);
        return &ptr_;
    }

    void** ReceiveVoid() {
        return reinterpret_cast<void**>(Receive());
    }

    template <class Query>
    HRESULT QueryInterface(Query** p) {
        assert(p != NULL);
        assert(ptr_ != NULL);
        return ptr_->QueryInterface(p);
    }

    HRESULT QueryInterface(const IID& iid, void** obj) {
        assert(obj != NULL);
        assert(ptr_ != NULL);
        return ptr_->QueryInterface(iid, obj);
    }

    HRESULT QueryFrom(IUnknown* object) {
        assert(object != NULL);
        return object->QueryInterface(Receive());
    }

    HRESULT CreateInstance(const CLSID& clsid, IUnknown* outer = NULL,
        DWORD context = CLSCTX_ALL) {
        assert(!ptr_);
        HRESULT hr = ::CoCreateInstance(clsid, outer, context, *id,
            reinterpret_cast<void**>(&ptr_));
        return hr;
    }

    bool IsSameObject(IUnknown* other) {
        if (!other && !ptr_)
            return true;

        if (!other || !ptr_)
            return false;

        ScopedComObject<IUnknown> my_identity;
        QueryInterface(my_identity.Receive());

        ScopedComObject<IUnknown> other_identity;
        other->QueryInterface(other_identity.Receive());

        return static_cast<IUnknown*>(my_identity) ==
            static_cast<IUnknown*>(other_identity);
    }

    IUnknownMethods* operator->() const {
        return reinterpret_cast<IUnknownMethods*>(ptr_);
    }

    using scoped_refptr<Interface, RefCounted>::operator=;

    static const IID& iid() {
        return *interface_id;
    }


};

// QueryInterface:
// |-- object: should be ScopedComObject<T>.ReceiveVoid();
// |-- module: not nullptr.
HRESULT QueryInterface(const HMODULE& module, const CLSID& clsid, const IID& iid, void** object);

#endif  // !#define (DIRECTX_SCOPED_COM_OBJECT_INCLUDE_H_ )