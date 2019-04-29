////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017 The Authors of ANT(http://ant.sh). All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
////////////////////////////////////////////////////////////////////////////////
#include "utils/scoped_object.h"

#if defined(COMPILER_MSVC)
// We usually use the _CrtDumpMemoryLeaks() with the DEBUGER and CRT library to
// check a memory leak.
#if defined(_DEBUG) && _MSC_VER > 1000  // VC++ DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#if defined(DEBUG_NEW)
#define new DEBUG_NEW
#endif  // DEBUG_NEW
#endif  // VC++ DEBUG
#endif  // defined(COMPILER_MSVC)

UTILS_ABSL_COMDAT const VARIANT ScopedVariant::kEmptyVariant = {{{VT_EMPTY}}};

inline ScopedPropVariant::ScopedPropVariant() { PropVariantInit(&pv_); }

inline ScopedPropVariant::~ScopedPropVariant() { Reset(); }

// Returns a pointer to the underlying PROPVARIANT for use as an out param in
// a function call.

inline PROPVARIANT* ScopedPropVariant::Receive() {
  assert(pv_.vt == VT_EMPTY);
  return &pv_;
}

// Clears the instance to prepare it for re-use (e.g., via Receive).

inline void ScopedPropVariant::Reset() {
  if (pv_.vt != VT_EMPTY) {
    HRESULT result = PropVariantClear(&pv_);
    assert(result == S_OK);
  }
}

inline const PROPVARIANT& ScopedPropVariant::get() const { return pv_; }

inline const PROPVARIANT* ScopedPropVariant::ptr() const { return &pv_; }

inline ScopedVariant::ScopedVariant() { var_.vt = VT_EMPTY; }

inline ScopedVariant::ScopedVariant(const wchar_t* str) {
  var_.vt = VT_EMPTY;
  Set(str);
}

// Creates a new VT_BSTR variant of a specified length.

inline ScopedVariant::ScopedVariant(const wchar_t* str, UINT length) {
  var_.vt = VT_BSTR;
  var_.bstrVal = ::SysAllocStringLen(str, length);
}

// Creates a new integral type variant and assigns the value to
// VARIANT.lVal (32 bit sized field).

inline ScopedVariant::ScopedVariant(int value, VARTYPE vt) {
  var_.vt = vt;
  var_.lVal = value;
}

// Creates a new double-precision type variant.  |vt| must be either VT_R8
// or VT_DATE.

inline ScopedVariant::ScopedVariant(double value, VARTYPE vt) {
  assert(vt == VT_R8 || vt == VT_DATE);
  var_.vt = vt;
  var_.dblVal = value;
}

// VT_DISPATCH

inline ScopedVariant::ScopedVariant(IDispatch* dispatch) {
  var_.vt = VT_EMPTY;
  Set(dispatch);
}

// VT_UNKNOWN

inline ScopedVariant::ScopedVariant(IUnknown* unknown) {
  var_.vt = VT_EMPTY;
  Set(unknown);
}

// SAFEARRAY

inline ScopedVariant::ScopedVariant(SAFEARRAY* safearray) {
  var_.vt = VT_EMPTY;
  Set(safearray);
}

// Copies the variant.

inline ScopedVariant::ScopedVariant(const VARIANT& var) {
  var_.vt = VT_EMPTY;
  Set(var);
}

inline ScopedVariant::~ScopedVariant() { ::VariantClear(&var_); }

inline VARTYPE ScopedVariant::type() const { return var_.vt; }

// Give ScopedVariant ownership over an already allocated VARIANT.

inline void ScopedVariant::Reset(const VARIANT& var) {
  if (&var_ != &var) {
    ::VariantClear(&var_);
    var_ = var;
  }
}

// Releases ownership of the VARIANT to the caller.

inline VARIANT ScopedVariant::Release() {
  VARIANT var = var_;
  var_.vt = VT_EMPTY;
  return var;
}

inline HRESULT ScopedVariant::Release(PROPVARIANT* var) {
  if (IsLeakableVarType(var->vt)) {
    auto result = ::PropVariantClear(var);
    if (FAILED(result)) return result;
  } else {
    var->vt = VT_EMPTY;
    var->wReserved1 = 0;
  }
  std::memcpy(var, &var_, sizeof(PROPVARIANT));
  return S_OK;
}

// Swap two ScopedVariant's.

inline void ScopedVariant::Swap(ScopedVariant& var) {
  VARIANT tmp = var_;
  var_ = var.var_;
  var.var_ = tmp;
}

// Returns a copy of the variant.

inline VARIANT ScopedVariant::Copy() const {
  VARIANT ret = {{{VT_EMPTY}}};
  ::VariantCopy(&ret, &var_);
  return ret;
}

// The return value is 0 if the variants are equal, 1 if this object is
// greater than |var|, -1 if it is smaller.

inline int ScopedVariant::Compare(const VARIANT& var, bool ignore_case) const {
  ULONG flags = ignore_case ? NORM_IGNORECASE : 0;
  HRESULT hr = ::VarCmp(const_cast<VARIANT*>(&var_), const_cast<VARIANT*>(&var),
                        LOCALE_USER_DEFAULT, flags);
  int ret = 0;
  switch (hr) {
    case VARCMP_LT:
      ret = -1;
      break;
    case VARCMP_GT:
    case VARCMP_NULL:
      ret = 1;
      break;
    default:
      // Equal.
      break;
  }
  return ret;
}

// Retrieves the pointer address.
// Used to receive a VARIANT as an out argument (and take ownership).
// The function DCHECKs on the current value being empty/null.
// Usage: GetVariant(var.receive());

inline VARIANT* ScopedVariant::Receive() {
  assert(!IsLeakableVarType(var_.vt));
  return &var_;
}

inline void ScopedVariant::Set(const wchar_t* str) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_BSTR;
  var_.bstrVal = ::SysAllocString(str);
}

// Setters for simple types.

inline void ScopedVariant::Set(int8_t i8) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_I1;
  var_.cVal = i8;
}

inline void ScopedVariant::Set(uint8_t ui8) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_UI1;
  var_.bVal = ui8;
}

inline void ScopedVariant::Set(int16_t i16) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_I2;
  var_.iVal = i16;
}

inline void ScopedVariant::Set(uint16_t ui16) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_UI2;
  var_.uiVal = ui16;
}

inline void ScopedVariant::Set(int32_t i32) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_I4;
  var_.lVal = i32;
}

inline void ScopedVariant::Set(uint32_t ui32) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_UI4;
  var_.ulVal = ui32;
}

inline void ScopedVariant::Set(int64_t i64) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_I8;
  var_.llVal = i64;
}

inline void ScopedVariant::Set(uint64_t ui64) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_UI8;
  var_.ullVal = ui64;
}

inline void ScopedVariant::Set(float r32) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_R4;
  var_.fltVal = r32;
}

inline void ScopedVariant::Set(double r64) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_R8;
  var_.dblVal = r64;
}

inline void ScopedVariant::Set(bool b) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_BOOL;
  var_.boolVal = b ? VARIANT_TRUE : VARIANT_FALSE;
}

// Creates a copy of |var| and assigns as this instance's value.
// Note that this is different from the Reset() method that's used to
// free the current value and assume ownership.

inline void ScopedVariant::Set(const VARIANT& var) {
  assert(!IsLeakableVarType(var_.vt));
  if (FAILED(::VariantCopy(&var_, &var))) {
    var_.vt = VT_EMPTY;
  }
}

// COM object setters

inline void ScopedVariant::Set(IDispatch* disp) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_DISPATCH;
  var_.pdispVal = disp;
  if (disp) disp->AddRef();
}

inline void ScopedVariant::Set(IUnknown* unk) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_UNKNOWN;
  var_.punkVal = unk;
  if (unk) unk->AddRef();
}

// SAFEARRAY support

inline void ScopedVariant::Set(SAFEARRAY* array) {
  assert(!IsLeakableVarType(var_.vt));
  if (SUCCEEDED(::SafeArrayGetVartype(array, &var_.vt))) {
    var_.vt |= VT_ARRAY;
    var_.parray = array;
  } else {
    var_.vt = VT_EMPTY;
  }
}

// Special setter for DATE since DATE is a double and we already have
// a setter for double.

inline void ScopedVariant::SetDate(DATE date) {
  assert(!IsLeakableVarType(var_.vt));
  var_.vt = VT_DATE;
  var_.date = date;
}

// Allows const access to the contained variant without DCHECKs etc.
// This support is necessary for the V_XYZ (e.g. V_BSTR) set of macros to
// work properly but still doesn't allow modifications since we want control
// over that.

inline const VARIANT* ScopedVariant::ptr() const { return &var_; }

// Like other scoped classes (e.g. scoped_refptr, ScopedBstr,
// Microsoft::WRL::ComPtr) we support the assignment operator for the type we
// wrap.
ScopedVariant& ScopedVariant::operator=(const VARIANT& var) {
  if (&var != &var_) {
    VariantClear(&var_);
    Set(var);
  }
  return *this;
}

// A hack to pass a pointer to the variant where the accepting
// function treats the variant as an input-only, read-only value
// but the function prototype requires a non const variant pointer.
// There's no DCHECK or anything here.  Callers must know what they're doing.
//
// The nature of this function is const, so we declare
// it as such and cast away the constness here.

inline VARIANT* ScopedVariant::AsInput() const {
  return const_cast<VARIANT*>(&var_);
}

// Allows the ScopedVariant instance to be passed to functions either by value
// or by const reference.
ScopedVariant::operator const VARIANT&() const { return var_; }

// Used as a debug check to see if we're leaking anything.

inline bool ScopedVariant::IsLeakableVarType(VARTYPE vt) {
  bool leakable = false;
  switch (vt & VT_TYPEMASK) {
    case VT_BSTR:
    case VT_DISPATCH:
    case VT_VARIANT:
    case VT_UNKNOWN:
    case VT_SAFEARRAY:
    case VT_VOID:
    case VT_PTR:
    case VT_CARRAY:
    case VT_USERDEFINED:
    case VT_LPSTR:
    case VT_LPWSTR:
    case VT_RECORD:
    case VT_INT_PTR:
    case VT_UINT_PTR:
    case VT_FILETIME:
    case VT_BLOB:
    case VT_STREAM:
    case VT_STORAGE:
    case VT_STREAMED_OBJECT:
    case VT_STORED_OBJECT:
    case VT_BLOB_OBJECT:
    case VT_VERSIONED_STREAM:
    case VT_BSTR_BLOB:
      leakable = true;
      break;
  }

  if (!leakable && (vt & VT_ARRAY) != 0) {
    leakable = true;
  }

  return leakable;
}