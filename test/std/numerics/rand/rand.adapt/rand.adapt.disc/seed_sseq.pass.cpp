//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t p, size_t r>
// class discard_block_engine

// template<class Sseq> void seed(Sseq& q);

#include <random>
#include <cassert>
#include "circular_sseq.h"

void
test1()
{
    circular_sseq sseq{3, 5, 7};
    std::ranlux24 e1;
    std::ranlux24 e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

void
test2()
{
    circular_sseq sseq{3, 5, 7};
    std::ranlux48 e1;
    std::ranlux48 e2(sseq);
    assert(e1 != e2);
    e1.seed(sseq);
    assert(e1 == e2);
}

int main()
{
    test1();
    test2();
}
