/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef DIRECTX_SCOPED_REF_OBJECT_INCLUDE_H_ 
#define DIRECTX_SCOPED_REF_OBJECT_INCLUDE_H_ 

namespace subtle {

class RefCounted {
public:
    bool OneRef() const;

protected:
    explicit RefCounted();
    virtual ~RefCounted();

    void AddRef() const;
    bool Release() const;

private:
    mutable int ref_count_ = 0;
    RefCounted(const RefCounted&) = delete;
    void operator=(const RefCounted&) = delete;
};

class AtomicRefCounted {
public:
    bool OneRef() const;

protected:
    explicit AtomicRefCounted();
    virtual ~AtomicRefCounted();

    void AddRef() const;
    bool Release() const;

private:
    mutable int ref_count_ = 0;
    AtomicRefCounted(const AtomicRefCounted&) = delete;
    void operator=(const AtomicRefCounted&) = delete;
};

} // subtle

template <typename T>
class RefObject : public subtle::RefCounted {
public:
    RefObject() {}

    void AddRef() const override {
        subtle::RefCounted::AddRef();
    }

    void Release() const {
        if (subtle::RefCounted::Release()) {
            delete static_cast<const T*>(this);
        }
    }

protected:
    virtual ~RefObject() {}

private:
    RefObject<T>(const RefObject<T>&) = delete;
    void operator=(const RefObject<T>&) = delete;
};

template<typename T>
class AtomicRefObject : public subtle::AtomicRefCounted {
public:
    AtomicRefObject() {}

    void AddRef() const {
        subtle::RefCountedThreadSafeBase::AddRef();
    }

    void Release() const {
        if (subtle::RefCountedThreadSafeBase::Release()) {
            delete static_cast<const T*>(this);
        }
    }
protected:
    virtual ~AtomicRefObject() {}

private:
    AtomicRefObject<T>(const AtomicRefObject<T>&) = delete;
    void operator=(const AtomicRefObject<T>&) = delete;
};

template<typename T, bool RefCounted = false>
class scoped_refptr {
public:
    scoped_refptr() {}

    scoped_refptr(T* p) : ptr_(p) {
        if (ptr_ && !RefCounted) ptr_->AddRef();
    }

    scoped_refptr(const scoped_refptr<T>& r) : ptr_(r.ptr_) {
        if (ptr_) ptr_->AddRef();
    }

    template <typename U>
    scoped_refptr(const scoped_refptr<U>& r) : ptr_(r.get()) {
        if (ptr_) ptr_->AddRef();
    }

    ~scoped_refptr() {
        if (ptr_) ptr_->Release();
    }

    T* get() const { return ptr_; }
    operator T*() const { return ptr_; }
    T* operator->() const {
        assert(ptr_ != NULL);
        return ptr_;
    }

    scoped_refptr<T>& operator=(T* p) {
        if (p) p->AddRef();
        T* old_ptr = ptr_;
        ptr_ = p;
        if (old_ptr) old_ptr->Release();
        return *this;
    }

    scoped_refptr<T>& operator=(const scoped_refptr<T>& r) {
        return *this = r.ptr_;
    }

    template <typename U>
    scoped_refptr<T>& operator=(const scoped_refptr<U>& r) {
        return *this = r.get();
    }

    void swap(T** pp) {
        T* p = ptr_;
        ptr_ = *pp;
        *pp = p;
    }

    void swap(scoped_refptr<T>& r) { swap(&r.ptr_); }

protected:
    T* ptr_ = nullptr;
};

template <typename T>
scoped_refptr<T> make_scoped_refptr(T* t) {
    return scoped_refptr<T>(t);
}


#endif  // !#define (DIRECTX_SCOPED_REF_OBJECT_INCLUDE_H_ )