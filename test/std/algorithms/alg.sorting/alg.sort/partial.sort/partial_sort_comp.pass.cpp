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
//   partial_sort(Iter first, Iter middle, Iter last, Compare comp);

#include <algorithm>
#include <vector>
#include <functional>
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

static const int max_size_test = 2000;
int input_array[max_size_test];

void
test_larger_sorts(int N, int M)
{
    assert(N != 0);
    assert(N >= M);
    int* array = input_array;
    for (int i = 0; i < N; ++i)
        array[i] = i;
    std::random_shuffle(array, array+N);
    std::partial_sort(array, array+M, array+N, std::greater<int>());
    for (int i = 0; i < M; ++i)
    {
        assert(i < N); // quiet analysis warnings
        assert(array[i] == N-i-1);
    }
}

void
test_larger_sorts(int N)
{
    test_larger_sorts(N, 0);
    test_larger_sorts(N, 1);
    test_larger_sorts(N, 2);
    test_larger_sorts(N, 3);
    test_larger_sorts(N, N/2-1);
    test_larger_sorts(N, N/2);
    test_larger_sorts(N, N/2+1);
    test_larger_sorts(N, N-2);
    test_larger_sorts(N, N-1);
    test_larger_sorts(N, N);
}

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
MoveOnly move_only_v[1000];
#endif

int main()
{
    {
    int i = 0;
    std::partial_sort(&i, &i, &i);
    assert(i == 0);
    test_larger_sorts(10);
    test_larger_sorts(256);
    test_larger_sorts(257);
    test_larger_sorts(499);
    test_larger_sorts(500);
    test_larger_sorts(997);
    test_larger_sorts(1000);
    test_larger_sorts(1009);
    }

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
    for (int i = 0; static_cast<std::size_t>(i) < std::size(move_only_v); ++i)
        move_only_v[i].reset(i);
    std::partial_sort(std::begin(move_only_v), std::begin(move_only_v) + std::size(move_only_v)/2, std::end(move_only_v), indirect_less());
    for (int i = 0; static_cast<std::size_t>(i) < std::size(move_only_v)/2; ++i)
        assert(*move_only_v[i] == i);
    }
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}
