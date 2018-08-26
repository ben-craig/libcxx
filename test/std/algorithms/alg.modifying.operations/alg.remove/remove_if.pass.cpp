//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter, Predicate<auto, Iter::value_type> Pred>
//   requires OutputIterator<Iter, RvalueOf<Iter::reference>::type>
//         && CopyConstructible<Pred>
//   Iter
//   remove_if(Iter first, Iter last, Pred pred);

#include <algorithm>
#include <functional>
#include <cassert>
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#include "MoveOnly.h"
#endif

#include "test_iterators.h"
#include "counting_predicates.hpp"

bool equal2 ( int i ) { return i == 2; }

template <class Iter>
void
test()
{
    int ia[] = {0, 1, 2, 3, 4, 2, 3, 4, 2};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
//     int* r = std::remove_if(ia, ia+sa, std::bind2nd(std::equal_to<int>(), 2));
    unary_counting_predicate<bool(*)(int), int> cp(equal2);
    int* r = std::remove_if(ia, ia+sa, std::ref(cp));
    assert(r == ia + sa-3);
    assert(ia[0] == 0);
    assert(ia[1] == 1);
    assert(ia[2] == 3);
    assert(ia[3] == 4);
    assert(ia[4] == 3);
    assert(ia[5] == 4);
    assert(cp.count() == sa);
}

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

struct pred
{
    bool operator()(const MoveOnly& i) {return *i == 2;}
};

template <class Iter>
void
test1()
{
    const unsigned sa = 9;
    MoveOnly ia[sa];
    ia[0].reset(0);
    ia[1].reset(1);
    ia[2].reset(2);
    ia[3].reset(3);
    ia[4].reset(4);
    ia[5].reset(2);
    ia[6].reset(3);
    ia[7].reset(4);
    ia[8].reset(2);
    Iter r = std::remove_if(Iter(ia), Iter(ia+sa), pred());
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
