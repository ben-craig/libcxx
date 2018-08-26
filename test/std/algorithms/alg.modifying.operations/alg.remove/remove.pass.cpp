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
//   requires OutputIterator<Iter, RvalueOf<Iter::reference>::type>
//         && HasEqualTo<Iter::value_type, T>
//   Iter
//   remove(Iter first, Iter last, const T& value);

#include <algorithm>
#include <cassert>
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#include "MoveOnly.h"
#endif

#include "test_iterators.h"

template <class Iter>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 2, 3, 4, 2};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    Iter r = std::remove(Iter(ia), Iter(ia+sa), 2);
    assert(base(r) == ia + sa-3);
    assert(ia[0] == 0);
    assert(ia[1] == 1);
    assert(ia[2] == 3);
    assert(ia[3] == 4);
    assert(ia[4] == 3);
    assert(ia[5] == 4);
}

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

template <class Iter>
void
test1()
{
    const unsigned sa = 9;
    MoveOnly ia[sa];
    ia[0].reset(0);
    ia[1].reset(1);
    ia[2].reset(-1);
    ia[3].reset(3);
    ia[4].reset(4);
    ia[5].reset(-1);
    ia[6].reset(3);
    ia[7].reset(4);
    ia[8].reset(-1);
    Iter r = std::remove(Iter(ia), Iter(ia+sa), MoveOnly(-1));
    assert(base(r) == ia + sa-3);
    assert(*ia[0] == 0);
    assert(*ia[1] == 1);
    assert(*ia[2] == 3);
    assert(*ia[3] == 4);
    assert(*ia[4] == 3);
    assert(*ia[5] == 4);
}

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

int main()
{
    test<forward_iterator<int*> >();
    test<bidirectional_iterator<int*> >();
    test<random_access_iterator<int*> >();
    test<int*>();

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

    test1<forward_iterator<MoveOnly*> >();
    test1<bidirectional_iterator<MoveOnly*> >();
    test1<random_access_iterator<MoveOnly*> >();
    test1<MoveOnly*>();

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}
