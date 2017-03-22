// -*- C++ -*-
//===--------------------- support/win32/float_win32.h -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_SUPPORT_WIN32_FLOAT_WIN32_H
#define _LIBCPP_SUPPORT_WIN32_FLOAT_WIN32_H
#include <__config>
#if !defined(_LIBCPP_MSVCRT)
#error "This header complements the Microsoft C Runtime library, and should not be included otherwise."
#endif

#include <math.h> // _copysignl, _copysignf

#if 0
inline float copysignf(float _Number, float _Sign) { return _copysignf(_Number, _Sign); }
#endif
inline long double copysignl(long double _Number, long double _Sign) { return _copysignl(_Number, _Sign); }

#endif // _LIBCPP_SUPPORT_WIN32_FLOAT_WIN32_H
