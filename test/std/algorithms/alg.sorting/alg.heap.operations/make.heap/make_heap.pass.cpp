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
//   requires ShuffleIterator<Iter> && LessThanComparable<Iter::value_type>
//   void
//   make_heap(Iter first, Iter last);

#include <algorithm>
#include <array>
#include <cassert>

std::array<int, 1000> ia;

void test(int N)
{
    assert(N <= ia.size());
    for (int i = 0; i < N; ++i)
        ia[i] = i;
    std::random_shuffle(ia.begin(), ia.begin()+N);
    std::make_heap(ia.begin(), ia.begin()+N);
    assert(std::is_heap(ia.begin(), ia.begin()+N));
}

int main()
{
    test(0);
    test(1);
    test(2);
    test(3);
    test(10);
    test(1000);
}
