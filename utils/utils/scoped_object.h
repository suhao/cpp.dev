/////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Copyright (c) 2018 The Authors of ANT(http:://ant.sh) . All Rights Reserved. 
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 
// 
/////////////////////////////////////////////////////////////////////////////////////////// 


#ifndef JUICE_SCOPED_OBJECT_INCLUDE_H_
#define JUICE_SCOPED_OBJECT_INCLUDE_H_

#include <algorithm>

#include <stdlib.h>
#include <unknwn.h>
#include <assert.h>
#include <Windows.h>
#include <OleAuto.h>

#include "utils.h"
#include "utils/basictypes.h"

static const int32 kExChangedStep = 1;

template<typename T>
class RefCounted {
public:
    RefCounted() {}

    auto AddRef() const {
        auto atom = InterlockedExchangeAdd(
            reinterpret_cast<volatile LONG*>(&ref_count_),
            static_cast<LONG>(kExChangedStep));
        return atom;
    }

    auto Release() const {
        auto step = -kExChangedStep;
        auto atom = InterlockedExchangeAdd(
            reinterpret_cast<volatile LONG*>(&ref_count_),
            static_cast<LONG>(step));
        if (0 == atom) {
            DeleteInternal(static_cast<const T*>(this));
        }
        return atom;
    }

    bool HasOneRef() const { return 1 = *(&ref_count_); }

protected:
    virtual ~RefCounted() {}
    
private:
    static void DeleteInternal(const T* x) { delete x; }
    mutable int32 ref_count_ = 0;
    DISALLOW_COPY_AND_ASSIGN(RefCounted);
};

template<class Interface, class Containter>
bool Query(Containter* containter, REFIID iid, void** obj) {
    if (iid == __uuidof(Interface)) {
        *obj = reinterpret_cast<Interface*>(containter);
        containter->AddRef();
        return true;
    }
    return false;
}

template<class Interface, class Containter>
bool Query(Containter* containter, const IID& id, REFIID iid, void** obj) {
    if (iid == id) {
        *obj = reinterpret_cast<Interface*>(containter);
        containter->AddRef();
        return true;
    }
    return false;
}


template<class Interface>
class ScopedComObject {
public:
    class IUnknownMethods : public Interface {
    private:
        STDMETHOD(QueryInterface)(REFIID iid, void** object) = 0;
        STDMETHOD_(ULONG, AddRef)() = 0;
        STDMETHOD_(ULONG, Release)() = 0;
    };

    ScopedComObject() {}
    ScopedComObject(std::nullptr_t) {}

    explicit ScopedComObject(Interface* p) : ptr_(p) {
        if (ptr_)
            ptr_->AddRef();
    }

    ScopedComObject(const ScopedComObject<Interface>& p)
        : ScopedComObject(p.ptr_) {}

    virtual ~ScopedComObject() {
        if (ptr_)
            ptr_->Release();
    }

    ScopedComObject& operator=(std::nullptr_t) {
        Release();
        return *this;
    }

    template <typename U>
    ScopedComObject& operator=(U *p) {
        ScopedComObject(p).swap(*this);
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

    ScopedComObject<Interface>& operator=(Interface* p) {
        if (ptr_ != p) { ScopedComObject(p).swap(*this); }
        return *this;
    }

    IUnknownMethods* operator->() const {
        return reinterpret_cast<IUnknownMethods*>(ptr_);
    }

    operator Interface*() const { return ptr_; }

    Interface* get() const { return ptr_; }

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

    Interface* const* QueryAdddress() const {
        assert(!ptr_);
        return &ptr_;
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

    void swap(Interface** pp) {
        Interface* p = ptr_;
        ptr_ = *pp;
        *pp = p;
    }

    void swap(ScopedComObject<Interface>& r) {
        swap(&r.ptr_);
    }

protected:
    Interface* ptr_ = nullptr;

};

template <typename T, typename Traits>
class ScopedGeneric {
  private:
    // This must be first since it's used inline below.
    struct Data : public Traits {
        explicit Data(const T &in) : generic(in) {}
        Data(const T &in, const Traits &other) : Traits(other), generic(in) {}
        T generic;
    };

  public:
    typedef T element_type;
    typedef Traits traits_type;

    ScopedGeneric() : data_(traits_type::InvalidValue()) {}
    explicit ScopedGeneric(const element_type &value) : data_(value) {}
    ScopedGeneric(const element_type &value, const traits_type &traits)
        : data_(value, traits) {}
    ScopedGeneric(ScopedGeneric<T, Traits> &&rvalue)
        : data_(rvalue.release(), rvalue.get_traits()) {}

    ~ScopedGeneric() { FreeIfNecessary(); }
    ScopedGeneric &operator=(ScopedGeneric<T, Traits> &&rvalue) {
        reset(rvalue.release());
        return *this;
    }
    void reset(const element_type &value = traits_type::InvalidValue()) {
        if (data_.generic != traits_type::InvalidValue() && data_.generic == value)
            abort();
        FreeIfNecessary();
        data_.generic = value;
    }
    void swap(ScopedGeneric &other) {
        if (&other == this) return;
        std::swap(static_cast<Traits &>(data_), static_cast<Traits &>(other.data_));
        std::swap(data_.generic, other.data_.generic);
    }

    element_type release() WARN_UNUSED_RESULT {
        element_type old_generic = data_.generic;
        data_.generic = traits_type::InvalidValue();
        return old_generic;
    }
    class Receiver {
      public:
        explicit Receiver(ScopedGeneric &parent) : scoped_generic_(&parent) {
            scoped_generic_->receiving_ = true;
        }

        ~Receiver() {
            if (scoped_generic_)  {
                scoped_generic_->reset(value_);
                scoped_generic_->receiving_ = false;
            }
        }

        Receiver(Receiver &&move) {
            scoped_generic_ = move.scoped_generic_;
            move.scoped_generic_ = nullptr;
        }

        Receiver &operator=(Receiver &&move) {
            scoped_generic_ = move.scoped_generic_;
            move.scoped_generic_ = nullptr;
        }

        // We hand out a pointer to a field in Receiver instead of directly to
        // ScopedGeneric's internal storage in order to make it so that users can't
        // accidentally silently break ScopedGeneric's invariants. This way, an
        // incorrect use-after-scope-exit is more detectable by ASan or static
        // analysis tools, as the pointer is only valid for the lifetime of the
        // Receiver, not the ScopedGeneric.
        T *get() {
            used_ = true;
            return &value_;
        }

      private:
        T value_ = Traits::InvalidValue();
        ScopedGeneric *scoped_generic_;
        bool used_ = false;
        DISALLOW_COPY_AND_ASSIGN(Receiver);
    };
    const element_type &get() const { return data_.generic; }
    bool is_valid() const { return data_.generic != traits_type::InvalidValue(); }
    bool operator==(const element_type &value) const { return data_.generic == value; }
    bool operator!=(const element_type &value) const { return data_.generic != value; }
    Traits &get_traits() { return data_; }
    const Traits &get_traits() const { return data_; }

  private:
    void FreeIfNecessary() {
        if (data_.generic != traits_type::InvalidValue()) {
            data_.Free(data_.generic);
            data_.generic = traits_type::InvalidValue();
        }
    }

    template <typename T2, typename Traits2>
    bool operator==(
        const ScopedGeneric<T2, Traits2> &p2) const;
    template <typename T2, typename Traits2>
    bool operator!=(
        const ScopedGeneric<T2, Traits2> &p2) const;

    Data data_;
    bool receiving_ = false;

    DISALLOW_COPY_AND_ASSIGN(ScopedGeneric);
};

template <class T, class Traits>
void swap(const ScopedGeneric<T, Traits> &a, const ScopedGeneric<T, Traits> &b) {
    a.swap(b);
}

template <class T, class Traits>
bool operator==(const T &value, const ScopedGeneric<T, Traits> &scoped) {
    return value == scoped.get();
}

template <class T, class Traits>
bool operator!=(const T &value, const ScopedGeneric<T, Traits> &scoped) {
    return value != scoped.get();
}

class UTILS_API ScopedVariant {
 public:
    // Declaration of a global variant variable that's always VT_EMPTY
    static const VARIANT kEmptyVariant;

    ScopedVariant();
    explicit ScopedVariant(const wchar_t* str);

    // Creates a new VT_BSTR variant of a specified length.
    ScopedVariant(const wchar_t* str, UINT length);

    // Creates a new integral type variant and assigns the value to
    // VARIANT.lVal (32 bit sized field).
    explicit ScopedVariant(int value, VARTYPE vt = VT_I4);

    // Creates a new double-precision type variant.  |vt| must be either VT_R8
    // or VT_DATE.
    explicit ScopedVariant(double value, VARTYPE vt = VT_R8);

    // VT_DISPATCH
    explicit ScopedVariant(IDispatch* dispatch);

    // VT_UNKNOWN
    explicit ScopedVariant(IUnknown* unknown);

    // SAFEARRAY
    explicit ScopedVariant(SAFEARRAY* safearray);

    // Copies the variant.
    explicit ScopedVariant(const VARIANT& var);

    virtual ~ScopedVariant();

    inline VARTYPE type() const;

    // Give ScopedVariant ownership over an already allocated VARIANT.
    void Reset(const VARIANT& var = kEmptyVariant);

    // Releases ownership of the VARIANT to the caller.
    VARIANT Release();

    HRESULT Release(PROPVARIANT* var);

    // Swap two ScopedVariant's.
    void Swap(ScopedVariant& var);

    // Returns a copy of the variant.
    VARIANT Copy() const;

    // The return value is 0 if the variants are equal, 1 if this object is
    // greater than |var|, -1 if it is smaller.
    int Compare(const VARIANT& var, bool ignore_case = false) const;

    // Retrieves the pointer address.
    // Used to receive a VARIANT as an out argument (and take ownership).
    // The function DCHECKs on the current value being empty/null.
    // Usage: GetVariant(var.receive());
    VARIANT* Receive();

    void Set(const wchar_t* str);

    // Setters for simple types.
    void Set(int8_t i8);
    void Set(uint8_t ui8);
    void Set(int16_t i16);
    void Set(uint16_t ui16);
    void Set(int32_t i32);
    void Set(uint32_t ui32);
    void Set(int64_t i64);
    void Set(uint64_t ui64);
    void Set(float r32);
    void Set(double r64);
    void Set(bool b);

    // Creates a copy of |var| and assigns as this instance's value.
    // Note that this is different from the Reset() method that's used to
    // free the current value and assume ownership.
    void Set(const VARIANT& var);

    // COM object setters
    void Set(IDispatch* disp);
    void Set(IUnknown* unk);

    // SAFEARRAY support
    void Set(SAFEARRAY* array);

    // Special setter for DATE since DATE is a double and we already have
    // a setter for double.
    void SetDate(DATE date);

    // Allows const access to the contained variant without DCHECKs etc.
    // This support is necessary for the V_XYZ (e.g. V_BSTR) set of macros to
    // work properly but still doesn't allow modifications since we want control
    // over that.
    const VARIANT* ptr() const;

    // Like other scoped classes (e.g. scoped_refptr, ScopedBstr,
    // Microsoft::WRL::ComPtr) we support the assignment operator for the type we
    // wrap.
    ScopedVariant& operator=(const VARIANT& var);

    // A hack to pass a pointer to the variant where the accepting
    // function treats the variant as an input-only, read-only value
    // but the function prototype requires a non const variant pointer.
    // There's no DCHECK or anything here.  Callers must know what they're doing.
    //
    // The nature of this function is const, so we declare
    // it as such and cast away the constness here.
    VARIANT* AsInput() const;

    // Allows the ScopedVariant instance to be passed to functions either by value
    // or by const reference.
    operator const VARIANT&() const;

    // Used as a debug check to see if we're leaking anything.
    static bool IsLeakableVarType(VARTYPE vt);

protected:
    VARIANT var_;

private:
    // Comparison operators for ScopedVariant are not supported at this point.
    // Use the Compare method instead.
    bool operator==(const ScopedVariant& var) const;
    bool operator!=(const ScopedVariant& var) const;
    DISALLOW_COPY_AND_ASSIGN(ScopedVariant);
};


class UTILS_API ScopedPropVariant {
public:
    ScopedPropVariant();

    virtual ~ScopedPropVariant();

    // Returns a pointer to the underlying PROPVARIANT for use as an out param in
    // a function call.
    PROPVARIANT* Receive();

    // Clears the instance to prepare it for re-use (e.g., via Receive).
    void Reset();
    const PROPVARIANT& get() const;
    const PROPVARIANT* ptr() const;

private:
    PROPVARIANT pv_;

    // Comparison operators for ScopedPropVariant are not supported at this point.
    bool operator==(const ScopedPropVariant&) const;
    bool operator!=(const ScopedPropVariant&) const;
    DISALLOW_COPY_AND_ASSIGN(ScopedPropVariant);
};





#endif // !#define (JUICE_SCOPED_OBJECT_INCLUDE_H_ )