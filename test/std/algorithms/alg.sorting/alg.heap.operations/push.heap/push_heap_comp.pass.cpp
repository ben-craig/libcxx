//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<RandomAccessIterator Iter>
//   requires ShuffleIterator<Iter>
//         && LessThanComparable<Iter::value_type>
//   void
//   push_heap(Iter first, Iter last);

#include <algorithm>
#include <functional>
#include <cassert>
#include <array>

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#include "MoveOnly.h"

struct indirect_less
{
    template <class P>
    bool operator()(const P& x, const P& y)
        {return *x < *y;}
};

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

std::array<int, 1000> ia;

void test()
{
    for (int i = 0; i < ia.size(); ++i)
        ia[i] = i;
    std::random_shuffle(ia.begin(), ia.end());
    for (int i = 0; i <= ia.size(); ++i)
    {
        std::push_heap(ia.begin(), ia.begin()+i, std::greater<int>());
        assert(std::is_heap(ia.begin(), ia.begin()+i, std::greater<int>()));
    }
}

std::array<MoveOnly, 1000> uia;
int main()
{
    test();

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
    for (int i = 0; i < uia.size(); ++i)
        uia[i].reset(i);
    std::random_shuffle(uia.begin(), uia.end());
    for (int i = 0; i <= uia.size(); ++i)
    {
        std::push_heap(uia.begin(), uia.begin()+i, indirect_less());
        assert(std::is_heap(uia.begin(), uia.begin()+i, indirect_less()));
    }
    }
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}
