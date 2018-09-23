//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, class T>
//   requires HasLess<Iter::value_type, T>
//   Iter
//   lower_bound(Iter first, Iter last, const T& value);

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>

#include "test_iterators.h"

template <class Iter, class T>
void
test(Iter first, Iter last, const T& value)
{
    Iter i = std::lower_bound(first, last, value);
    for (Iter j = first; j != i; ++j)
        assert(*j < value);
    for (Iter j = i; j != last; ++j)
        assert(!(*j < value));
}

const unsigned N = 1000;
const int M = 10;
std::array<int, N> v;

template <class Iter>
void
test()
{
    int x = 0;
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        v[i] = x;
        if (++x == M)
            x = 0;
    }
    std::sort(v.begin(), v.end());
    for (x = 0; x <= M; ++x)
        test(Iter(v.data()), Iter(v.data()+v.size()), x);
}

int main()
{
    int d[] = {0, 1, 2, 3};
    for (int* e = d; e <= d+4; ++e)
        for (int x = -1; x <= 4; ++x)
            test(d, e, x);

    test<forward_iterator<const int*> >();
    test<bidirectional_iterator<const int*> >();
    test<random_access_iterator<const int*> >();
    test<const int*>();
}
