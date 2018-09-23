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
//   pop_heap(Iter first, Iter last);

#include <algorithm>
#include <cassert>
#include <array>

std::array<int, 1000> ia;

void test(int N)
{
    assert( N <= ia.size() );
    for (int i = 0; i < N; ++i)
        ia[i] = i;
    std::random_shuffle(ia.begin(), ia.begin()+N);
    std::make_heap(ia.begin(), ia.begin()+N);
    for (int i = N; i > 0; --i)
    {
        std::pop_heap(ia.begin(), ia.begin()+i);
        assert(std::is_heap(ia.begin(), ia.begin()+i-1));
    }
    std::pop_heap(ia.begin(), ia.begin());
}

int main()
{
    test(1000);
}
