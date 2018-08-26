//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<InputIterator InIter, typename OutIter>
//   requires OutputIterator<OutIter, RvalueOf<InIter::reference>::type>
//   OutIter
//   move(InIter first, InIter last, OutIter result);

#include <algorithm>
#include <cassert>
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#include "MoveOnly.h"
#endif

#include "test_iterators.h"

template <class InIter, class OutIter>
void
test()
{
    const unsigned N = 1000;
    int ia[N];
    for (unsigned i = 0; i < N; ++i)
        ia[i] = i;
    int ib[N] = {0};

    OutIter r = std::move(InIter(ia), InIter(ia+N), OutIter(ib));
    assert(base(r) == ib+N);
    for (unsigned i = 0; i < N; ++i)
        assert(ia[i] == ib[i]);
}

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

template <class InIter, class OutIter>
void
test1()
{
    const unsigned N = 100;
    MoveOnly ia[N];
    for (unsigned i = 0; i < N; ++i)
        ia[i].reset(i);
    MoveOnly ib[N];

    OutIter r = std::move(InIter(ia), InIter(ia+N), OutIter(ib));
    assert(base(r) == ib+N);
    for (unsigned i = 0; i < N; ++i)
        assert(*ib[i] == static_cast<int>(i));
}

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

int main()
{
    test<input_iterator<const int*>, output_iterator<int*> >();
    test<input_iterator<const int*>, input_iterator<int*> >();
    test<input_iterator<const int*>, forward_iterator<int*> >();
    test<input_iterator<const int*>, bidirectional_iterator<int*> >();
    test<input_iterator<const int*>, random_access_iterator<int*> >();
    test<input_iterator<const int*>, int*>();

    test<forward_iterator<const int*>, output_iterator<int*> >();
    test<forward_iterator<const int*>, input_iterator<int*> >();
    test<forward_iterator<const int*>, forward_iterator<int*> >();
    test<forward_iterator<const int*>, bidirectional_iterator<int*> >();
    test<forward_iterator<const int*>, random_access_iterator<int*> >();
    test<forward_iterator<const int*>, int*>();

    test<bidirectional_iterator<const int*>, output_iterator<int*> >();
    test<bidirectional_iterator<const int*>, input_iterator<int*> >();
    test<bidirectional_iterator<const int*>, forward_iterator<int*> >();
    test<bidirectional_iterator<const int*>, bidirectional_iterator<int*> >();
    test<bidirectional_iterator<const int*>, random_access_iterator<int*> >();
    test<bidirectional_iterator<const int*>, int*>();

    test<random_access_iterator<const int*>, output_iterator<int*> >();
    test<random_access_iterator<const int*>, input_iterator<int*> >();
    test<random_access_iterator<const int*>, forward_iterator<int*> >();
    test<random_access_iterator<const int*>, bidirectional_iterator<int*> >();
    test<random_access_iterator<const int*>, random_access_iterator<int*> >();
    test<random_access_iterator<const int*>, int*>();

    test<const int*, output_iterator<int*> >();
    test<const int*, input_iterator<int*> >();
    test<const int*, forward_iterator<int*> >();
    test<const int*, bidirectional_iterator<int*> >();
    test<const int*, random_access_iterator<int*> >();
    test<const int*, int*>();

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    test1<input_iterator<MoveOnly*>, output_iterator<MoveOnly*> >();
    test1<input_iterator<MoveOnly*>, input_iterator<MoveOnly*> >();
    test1<input_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<input_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<input_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<input_iterator<MoveOnly*>, MoveOnly*>();

    test1<forward_iterator<MoveOnly*>, output_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, input_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<forward_iterator<MoveOnly*>, MoveOnly*>();

    test1<bidirectional_iterator<MoveOnly*>, output_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, input_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*>, MoveOnly*>();

    test1<random_access_iterator<MoveOnly*>, output_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, input_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, forward_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, bidirectional_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, random_access_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*>, MoveOnly*>();

    test1<MoveOnly*, output_iterator<MoveOnly*> >();
    test1<MoveOnly*, input_iterator<MoveOnly*> >();
    test1<MoveOnly*, forward_iterator<MoveOnly*> >();
    test1<MoveOnly*, bidirectional_iterator<MoveOnly*> >();
    test1<MoveOnly*, random_access_iterator<MoveOnly*> >();
    test1<MoveOnly*, MoveOnly*>();
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}
