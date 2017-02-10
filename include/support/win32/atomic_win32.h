// -*- C++ -*-
//===--------------------- support/win32/atomic_win32.h -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#pragma once
#include <intrin.h>

#ifndef _LIBCPP_ATOMIC
#error "atomic_win32.h should only be included inside <atomic>"
#endif
_LIBCPP_BEGIN_NAMESPACE_STD

namespace __msvc_atomic {
	__forceinline void __MemoryBarrier()
	{
		volatile long __Barrier;
		_InterlockedExchange(&__Barrier, 0);
	}

	template <typename _Tp>
	struct __msvc_atomic_t {
		typedef _Tp type;

		static_assert(is_trivially_copyable<_Tp>::value,
			"std::atomic<Tp> requires that 'Tp' be a trivially copyable type");

		_LIBCPP_INLINE_VISIBILITY
			__msvc_atomic_t() _NOEXCEPT = default;
		_LIBCPP_CONSTEXPR explicit __msvc_atomic_t(_Tp value) _NOEXCEPT
			: __a_value(value) {}
		_Tp __a_value;
  };
#define _Atomic(x) __msvc_atomic::__msvc_atomic_t<x>
  
  template <size_t _Size> struct __x86_ops {}; //TODO lock shard
  
#pragma warning(push)
#pragma warning(disable: 4800) // bool performance warning

  template <> struct __x86_ops<1> {
    typedef volatile char *__ptr_t;
    typedef char __val_t;
    
    template <typename _Tp>
    static _Tp __exchange(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedExchange8((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __cmp_exchange(volatile _Tp *__p, _Tp __val, _Tp __expected) {
      return (_Tp)(
        _InterlockedCompareExchange8((__ptr_t)__p, (__val_t)__val, (__val_t)__expected));
    }
    template <typename _Tp>
    static _Tp __add(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAdd8((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __and(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAnd8((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __or(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedOr8((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __xor(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedXor8((__ptr_t)__p, (__val_t)__val));
    }
  };
#pragma warning(pop) // bool performance warning

  template <> struct __x86_ops<2> {
    typedef volatile short *__ptr_t;
    typedef short __val_t;
    
    template <typename _Tp>
    static _Tp __exchange(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedExchange16((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __cmp_exchange(volatile _Tp *__p, _Tp __val, _Tp __expected) {
      return (_Tp)(
        _InterlockedCompareExchange16((__ptr_t)__p, (__val_t)__val, (__val_t)__expected));
    }
    template <typename _Tp>
    static _Tp __add(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAdd16((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __and(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAnd16((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __or(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedOr16((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __xor(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedXor16((__ptr_t)__p, (__val_t)__val));
    }
  };
  template <> struct __x86_ops<4> {
    typedef volatile long *__ptr_t;
    typedef long __val_t;
    
    template <typename _Tp>
    static _Tp __exchange(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedExchange((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __cmp_exchange(volatile _Tp *__p, _Tp __val, _Tp __expected) {
      return (_Tp)(
        _InterlockedCompareExchange((__ptr_t)__p, (__val_t)__val, (__val_t)__expected));
    }
    template <typename _Tp>
    static _Tp __add(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAdd((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __and(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAnd((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __or(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedOr((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __xor(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedXor((__ptr_t)__p, (__val_t)__val));
    }
  };
  template <> struct __x86_ops<8> {
    typedef volatile __int64 *__ptr_t;
    typedef __int64 __val_t;
    
    template <typename _Tp>
    static _Tp __exchange(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedExchange64((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __cmp_exchange(volatile _Tp *__p, _Tp __val, _Tp __expected) {
      return (_Tp)(
        _InterlockedCompareExchange64((__ptr_t)__p, (__val_t)__val, (__val_t)__expected));
    }
    template <typename _Tp>
    static _Tp __add(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAdd64((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __and(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedAnd64((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __or(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedOr64((__ptr_t)__p, (__val_t)__val));
    }
    template <typename _Tp>
    static _Tp __xor(volatile _Tp *__p, _Tp __val) {
      return (_Tp)(
        _InterlockedXor64((__ptr_t)__p, (__val_t)__val));
    }
  };
} // namespace __msvc_atomic

template <typename _Tp>
static inline
typename enable_if<
	is_assignable<volatile typename _Tp::type, _Tp>::value>::type
	__c11_atomic_init(volatile _Atomic(_Tp)* __a, _Tp __val) {
	__a->__a_value = __val;
}

template <typename _Tp>
static inline
typename enable_if<
	!is_assignable<volatile typename _Tp::type, _Tp>::value &&
	is_assignable<typename _Tp::type, _Tp>::value>::type
	__c11_atomic_init(volatile _Atomic(_Tp)* __a, _Tp __val) {
	// [atomics.types.generic]p1 guarantees _Tp is trivially copyable. Because
	// the default operator= in an object is not volatile, a byte-by-byte copy
	// is required.
	volatile char* to = reinterpret_cast<volatile char*>(&__a->__a_value);
	volatile char* end = to + sizeof(_Tp);
	char* from = reinterpret_cast<char*>(&__val);
	while (to != end) {
		*to++ = *from++;
	}
}

template <typename _Tp>
static inline void __c11_atomic_init(_Atomic(_Tp)* __a, _Tp __val) {
	__a->__a_value = __val;
}

static inline void __c11_atomic_thread_fence(memory_order __order) {
	if (__order == memory_order_seq_cst)
		__msvc_atomic::__MemoryBarrier();
	else
		_ReadWriteBarrier();
	// TODO: docs suck on _ReadBarrier and _WriteBarrier.  I suspect
	// that _WriteBarrier could be used for release and _ReadBarrier
	// for acquire, but I couldn't gather that from the docs.
}

static inline void __c11_atomic_signal_fence(memory_order /*__order*/) {
	_ReadWriteBarrier();
}

template <typename _Tp>
static inline void __c11_atomic_store(volatile _Atomic(_Tp)* __a, _Tp __val,
	memory_order __order) {
	if (__order == memory_order_seq_cst) {
		__msvc_atomic::__x86_ops<sizeof(_Tp)>::__exchange(&__a->__a_value, __val);
		return;
	}
	if (__order != memory_order_relaxed) {
		_ReadWriteBarrier();
	}
  __a->__a_value = __val;
}

template <typename _Tp>
static inline void __c11_atomic_store(_Atomic(_Tp)* __a, _Tp __val,
	memory_order __order) {
	if (__order == memory_order_seq_cst) {
		__msvc_atomic::__x86_ops<sizeof(_Tp)>::__exchange(&__a->__a_value, __val);
		return;
	}
	if (__order != memory_order_relaxed) {
		_ReadWriteBarrier();
	}
  __a->__a_value = __val;
}

template <typename _Tp>
static inline _Tp __c11_atomic_load(volatile _Atomic(_Tp)* __a,
	memory_order __order) {
	if (__order == memory_order_seq_cst) {
		_ReadWriteBarrier();
	}
	_Tp __ret = __a->__a_value;
	if (__order != memory_order_relaxed) {
		_ReadWriteBarrier();
	}
	return __ret;
}

template <typename _Tp>
static inline _Tp __c11_atomic_load(_Atomic(_Tp)* __a, memory_order __order) {
	if (__order == memory_order_seq_cst) {
		_ReadWriteBarrier();
	}
	_Tp __ret = __a->__a_value;
	if (__order != memory_order_relaxed) {
		_ReadWriteBarrier();
	}
	return __ret;
}

template <typename _Tp>
static inline _Tp __c11_atomic_exchange(volatile _Atomic(_Tp)* __a,
	_Tp __value, memory_order /*__order*/) {
	return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__exchange(&__a->__a_value, __value);
}

template <typename _Tp>
static inline _Tp __c11_atomic_exchange(_Atomic(_Tp)* __a, _Tp __value,
	memory_order /*__order*/) {
	return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__exchange(&__a->__a_value, __value);
}

template <typename _Tp>
static inline bool __c11_atomic_compare_exchange_strong(
	volatile _Atomic(_Tp)* __a, _Tp* __expected, _Tp __value,
	memory_order /*__success*/, memory_order /*__failure*/) {
  _Tp __exp = *__expected;
  _Tp __old = __msvc_atomic::__x86_ops<sizeof(_Tp)>::__cmp_exchange(&__a->__a_value, __value, __exp);
  if(__old == __exp)
    return true;
  *__expected = __old;
  return false;
}

template <typename _Tp>
static inline bool __c11_atomic_compare_exchange_strong(
	_Atomic(_Tp)* __a, _Tp* __expected, _Tp __value, memory_order /*__success*/,
	memory_order /*__failure*/) {
  _Tp __exp = *__expected;
  _Tp __old = __msvc_atomic::__x86_ops<sizeof(_Tp)>::__cmp_exchange(&__a->__a_value, __value, __exp);
  if(__old == __exp)
    return true;
  *__expected = __old;
  return false;
}

template <typename _Tp>
static inline bool __c11_atomic_compare_exchange_weak(
	volatile _Atomic(_Tp)* __a, _Tp* __expected, _Tp __value,
	memory_order /*__success*/, memory_order /*__failure*/) {
  _Tp __exp = *__expected;
  _Tp __old = __msvc_atomic::__x86_ops<sizeof(_Tp)>::__cmp_exchange(&__a->__a_value, __value, __exp);
  if(__old == __exp)
    return true;
  *__expected = __old;
  return false;
}

template <typename _Tp>
static inline bool __c11_atomic_compare_exchange_weak(
	_Atomic(_Tp)* __a, _Tp* __expected, _Tp __value, memory_order /*__success*/,
	memory_order /*__failure*/) {
  _Tp __exp = *__expected;
  _Tp __old = __msvc_atomic::__x86_ops<sizeof(_Tp)>::__cmp_exchange(&__a->__a_value, __value, __exp);
  if(__old == __exp)
    return true;
  *__expected = __old;
  return false;
}

template <typename _Tp>
struct __skip_amt { enum { value = 1 }; };

template <typename _Tp>
struct __skip_amt<_Tp*> { enum { value = sizeof(_Tp) }; };

// FIXME: Haven't figured out what the spec says about using arrays with
// atomic_fetch_add. Force a failure rather than creating bad behavior.
template <typename _Tp>
struct __skip_amt<_Tp[]> { };
template <typename _Tp, int n>
struct __skip_amt<_Tp[n]> { };

template <typename _Tp, typename _Td>
static inline _Tp __c11_atomic_fetch_add(volatile _Atomic(_Tp)* __a,
	_Td __delta, memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__add(
    &__a->__a_value, 
    __delta * __skip_amt<_Tp>::value); 
}

template <typename _Tp, typename _Td>
static inline _Tp __c11_atomic_fetch_add(_Atomic(_Tp)* __a, _Td __delta,
	memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__add(
    &__a->__a_value, 
    __delta * __skip_amt<_Tp>::value); 
}

template <typename _Tp, typename _Td>
static inline _Tp __c11_atomic_fetch_sub(volatile _Atomic(_Tp)* __a,
	_Td __delta, memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__add(
    &__a->__a_value, 
    __delta * -__skip_amt<_Tp>::value); 
}

template <typename _Tp, typename _Td>
static inline _Tp __c11_atomic_fetch_sub(_Atomic(_Tp)* __a, _Td __delta,
	memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__add(
    &__a->__a_value, 
    __delta * -__skip_amt<_Tp>::value); 
}

template <typename _Tp>
static inline _Tp __c11_atomic_fetch_and(volatile _Atomic(_Tp)* __a,
	_Tp __pattern, memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__and(&__a->__a_value, __pattern);
}

template <typename _Tp>
static inline _Tp __c11_atomic_fetch_and(_Atomic(_Tp)* __a,
	_Tp __pattern, memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__and(&__a->__a_value, __pattern);
}

template <typename _Tp>
static inline _Tp __c11_atomic_fetch_or(volatile _Atomic(_Tp)* __a,
	_Tp __pattern, memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__or(&__a->__a_value, __pattern);
}

template <typename _Tp>
static inline _Tp __c11_atomic_fetch_or(_Atomic(_Tp)* __a, _Tp __pattern,
	memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__or(&__a->__a_value, __pattern);
}

template <typename _Tp>
static inline _Tp __c11_atomic_fetch_xor(volatile _Atomic(_Tp)* __a,
	_Tp __pattern, memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__xor(&__a->__a_value, __pattern);
}

template <typename _Tp>
static inline _Tp __c11_atomic_fetch_xor(_Atomic(_Tp)* __a, _Tp __pattern,
	memory_order /*__order*/) {
  return __msvc_atomic::__x86_ops<sizeof(_Tp)>::__xor(&__a->__a_value, __pattern);
}

_LIBCPP_END_NAMESPACE_STD
