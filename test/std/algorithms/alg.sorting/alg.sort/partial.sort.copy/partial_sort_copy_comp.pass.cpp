//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter, RandomAccessIterator RAIter, class Compare>
//   requires ShuffleIterator<RAIter>
//         && OutputIterator<RAIter, InIter::reference>
//         && Predicate<Compare, InIter::value_type, RAIter::value_type>
//         && StrictWeakOrder<Compare, RAIter::value_type>}
//         && CopyConstructible<Compare>
//   RAIter
//   partial_sort_copy(InIter first, InIter last,
//                     RAIter result_first, RAIter result_last, Compare comp);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_iterators.h"

static const int max_size_test = 10000;
int input_array[max_size_test];
int output_array[max_size_test];

template <class Iter>
void
test_larger_sorts(int N, int M)
{
    assert(M < max_size_test);
    if(M >= max_size_test)
        return;
    assert(N < max_size_test);
    if(N >= max_size_test)
        return;

    int* input = input_array;
    int* output = output_array;
    for (int i = 0; i < N; ++i)
        input[i] = i;
    std::random_shuffle(input, input+N);
    int* r = std::partial_sort_copy(Iter(input), Iter(input+N), output, output+M,
                                    std::greater<int>());
    int* e = output + std::min(N, M);
    assert(r == e);
    int i = 0;
    for (int* x = output; x < e; ++x, ++i)
        assert(*x == N-i-1);
}

template <class Iter>
void
test_larger_sorts(int N)
{
    test_larger_sorts<Iter>(N, 0);
    test_larger_sorts<Iter>(N, 1);
    test_larger_sorts<Iter>(N, 2);
    test_larger_sorts<Iter>(N, 3);
    test_larger_sorts<Iter>(N, N/2-1);
    test_larger_sorts<Iter>(N, N/2);
    test_larger_sorts<Iter>(N, N/2+1);
    test_larger_sorts<Iter>(N, N-2);
    test_larger_sorts<Iter>(N, N-1);
    test_larger_sorts<Iter>(N, N);
    test_larger_sorts<Iter>(N, N+1000);
}

template <class Iter>
void
test()
{
    test_larger_sorts<Iter>(0, 100);
    test_larger_sorts<Iter>(10);
    test_larger_sorts<Iter>(256);
    test_larger_sorts<Iter>(257);
    test_larger_sorts<Iter>(499);
    test_larger_sorts<Iter>(500);
    test_larger_sorts<Iter>(997);
    test_larger_sorts<Iter>(1000);
    test_larger_sorts<Iter>(1009);
}

int main()
{
    int i = 0;
    std::partial_sort_copy(&i, &i, &i, &i+5);
    assert(i == 0);
    test<input_iterator<const int*> >();
    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
}
