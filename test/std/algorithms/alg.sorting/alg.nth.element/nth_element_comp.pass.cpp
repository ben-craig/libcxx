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
//   nth_element(Iter first, Iter nth, Iter last, Compare comp);

#include <algorithm>
#include <functional>
#include <vector>
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

int scratch_array[10000];

void
test_one(int N, int M)
{
    assert(N != 0);
    assert(M < N);
    int* array = scratch_array;
    for (int i = 0; i < N; ++i)
        array[i] = i;
    std::random_shuffle(array, array+N);
    std::nth_element(array, array+M, array+N, std::greater<int>());
    assert(array[M] == N-M-1);
    std::nth_element(array, array+N, array+N, std::greater<int>()); // begin, end, end
}

void
test(int N)
{
    test_one(N, 0);
    test_one(N, 1);
    test_one(N, 2);
    test_one(N, 3);
    test_one(N, N/2-1);
    test_one(N, N/2);
    test_one(N, N/2+1);
    test_one(N, N-3);
    test_one(N, N-2);
    test_one(N, N-1);
}

MoveOnly scratch_move[1000];

int main()
{
    int d = 0;
    std::nth_element(&d, &d, &d);
    assert(d == 0);
    test(256);
    test(257);
    test(499);
    test(500);
    test(997);
    test(1000);
    test(1009);

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
    for (int i = 0; static_cast<std::size_t>(i) < std::size(scratch_move); ++i)
        scratch_move[i].reset(i);
    std::nth_element(scratch_move, scratch_move + std::size(scratch_move)/2, std::end(scratch_move), indirect_less());
    assert(static_cast<std::size_t>(*scratch_move[std::size(scratch_move)/2]) == std::size(scratch_move)/2);
    }
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}
