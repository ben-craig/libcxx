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
#include <cassert>

int scratch_array[1000];
void test(int N)
{
    int* ia = scratch_array;
    for (int i = 0; i < N; ++i)
        ia[i] = i;
    std::random_shuffle(ia, ia+N);
    for (int i = 0; i <= N; ++i)
    {
        std::push_heap(ia, ia+i);
        assert(std::is_heap(ia, ia+i));
    }
}

int main()
{
    test(1000);
}
