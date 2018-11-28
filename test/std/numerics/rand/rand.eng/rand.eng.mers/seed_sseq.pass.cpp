//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t, UIntType c, size_t l, UIntType f>
// class mersenne_twister_engine;

// template<class Sseq> void seed(Sseq& q);

#include <random>
#include <cassert>
#include "circular_sseq.h"

void
test1()
{
    circular_sseq sseq{3, 5, 7};
    std::mt19937 e1;
    std::mt19937 e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

void
test2()
{
    circular_sseq sseq{3, 5, 7};
    std::mt19937_64 e1;
    std::mt19937_64 e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

int main()
{
    test1();
    test2();
}
