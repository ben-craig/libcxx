//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t k>
// class shuffle_order_engine

// shuffle_order_engine& operator=(const shuffle_order_engine&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::knuth_b E;
    E e1(2);
    e1();
    E e2(5);
    e2 = e1;
    assert(e1 == e2);
    assert(e1() == e2());
    E::result_type k = e1();
    assert(e1 != e2);
    assert(e2() == k);
    assert(e1 == e2);
}

int main()
{
    test1();
}
