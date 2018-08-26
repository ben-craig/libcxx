//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter1, ForwardIterator Iter2>
//   requires HasSwap<Iter1::reference, Iter2::reference>
//   Iter2
//   swap_ranges(Iter1 first1, Iter1 last1, Iter2 first2);

#include <algorithm>
#include <cassert>
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#include "MoveOnly.h"
#endif

#include "test_iterators.h"

template<class Iter1, class Iter2>
void
test()
{
    int i[3] = {1, 2, 3};
    int j[3] = {4, 5, 6};
    Iter2 r = std::swap_ranges(Iter1(i), Iter1(i+3), Iter2(j));
    assert(base(r) == j+3);
    assert(i[0] == 4);
    assert(i[1] == 5);
    assert(i[2] == 6);
    assert(j[0] == 1);
    assert(j[1] == 2);
    assert(j[2] == 3);
}

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

template<class Iter1, class Iter2>
void
test1()
{
    MoveOnly i[3];
    for (int k = 0; k < 3; ++k)
        i[k].reset(k+1);
    MoveOnly j[3];
    for (int k = 0; k < 3; ++k)
        j[k].reset(k+4);
    Iter2 r = std::swap_ranges(Iter1(i), Iter1(i+3), Iter2(j));
    assert(base(r) == j+3);
    assert(*i[0] == 4);
    assert(*i[1] == 5);
    assert(*i[2] == 6);
    assert(*j[0] == 1);
    assert(*j[1] == 2);
    assert(*j[2] == 3);
}

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

void test2()
{
    {
    int src[2][2]      = {{0, 1}, {2, 3}};
    decltype(src) dest = {{9, 8}, {7, 6}};

    std::swap(src, dest);

    assert ( src[0][0] == 9 );
    assert ( src[0][1] == 8 );
    assert ( src[1][0] == 7 );
    assert ( src[1][1] == 6 );

    assert ( dest[0][0] == 0 );
    assert ( dest[0][1] == 1 );
    assert ( dest[1][0] == 2 );
    assert ( dest[1][1] == 3 );
    }

    {
    int src[3][3]      = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
    decltype(src) dest = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};

    std::swap(src, dest);

    assert ( src[0][0] == 9 );
    assert ( src[0][1] == 8 );
    assert ( src[0][2] == 7 );
    assert ( src[1][0] == 6 );
    assert ( src[1][1] == 5 );
    assert ( src[1][2] == 4 );
    assert ( src[2][0] == 3 );
    assert ( src[2][1] == 2 );
    assert ( src[2][2] == 1 );

    assert ( dest[0][0] == 0 );
    assert ( dest[0][1] == 1 );
    assert ( dest[0][2] == 2 );
    assert ( dest[1][0] == 3 );
    assert ( dest[1][1] == 4 );
    assert ( dest[1][2] == 5 );
    assert ( dest[2][0] == 6 );
    assert ( dest[2][1] == 7 );
    assert ( dest[2][2] == 8 );
    }
}

int main()
{
    test<forward_iterator<int*>, forward_iterator<int*> >();
    test<forward_iterator<int*>, bidirectional_iterator<int*> >();
    test<forward_iterator<int*>, random_access_iterator<int*> >();
    test<forward_iterator<int*>, int*>();

    test<bidirectional_iterator<int*>, forward_iterator<int*> >();
    test<bidirectional_iterator<int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<int*>, int*>();

    test<random_access_iterator<int*>, forward_iterator<int*> >();
    test<random_access_iterator<int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<int*>, random_access_iterator<int*> >();
    test<random_access_iterator<int*>, int*>();

    test<int*, forward_iterator<int*> >();
    test<int*, bidirectional_iterator<int*> >();
    test<int*, random_access_iterator<int*> >();
    test<int*, int*>();

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

    test1<forward_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, MoveOnly*>();

    test1<bidirectional_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, MoveOnly*>();

    test1<random_access_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, MoveOnly*>();

    test1<MoveOnly*, forward_iterator<MoveOnly*> >();
    test1<MoveOnly*, bidirectional_iterator<MoveOnly*> >();
    test1<MoveOnly*, random_access_iterator<MoveOnly*> >();
    test1<MoveOnly*, MoveOnly*>();

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

    test2();
}
