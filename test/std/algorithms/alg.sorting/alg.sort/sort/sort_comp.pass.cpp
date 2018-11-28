//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<RandomAccessIterator Iter, StrictWeakOrder<auto, Iter::value_type> Compare>
//   requires ShuffleIterator<Iter>
//         && CopyConstructible<Compare>
//   void
//   sort(Iter first, Iter last, Compare comp);

#include <algorithm>
#include <functional>
#include <array>
#include <cassert>
#include <cstddef>
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#include "MoveOnly.h"

struct indirect_less
{
    template <class P>
    bool operator()(const P& x, const P& y)
        {return *x < *y;}
};

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

std::array<int, 1000> scratch_arr;
std::array<MoveOnly, 1000> scratch_move;

int main()
{
    {
    for (int i = 0; static_cast<std::size_t>(i) < scratch_arr.size(); ++i)
        scratch_arr[i] = i;
    std::sort(scratch_arr.begin(), scratch_arr.end(), std::greater<int>());
    std::reverse(scratch_arr.begin(), scratch_arr.end());
    assert(std::is_sorted(scratch_arr.begin(), scratch_arr.end()));
    }

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
    for (int i = 0; static_cast<std::size_t>(i) < scratch_move.size(); ++i)
        scratch_move[i].reset(i);
    std::sort(scratch_move.begin(), scratch_move.end(), indirect_less());
    assert(std::is_sorted(scratch_move.begin(), scratch_move.end(), indirect_less()));
    assert(*scratch_move[0] == 0);
    assert(*scratch_move[1] == 1);
    assert(*scratch_move[2] == 2);
    }
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}
