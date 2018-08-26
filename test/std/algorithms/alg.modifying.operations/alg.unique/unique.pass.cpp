//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter>
//   requires OutputIterator<Iter, Iter::reference>
//         && EqualityComparable<Iter::value_type>
//   Iter
//   unique(Iter first, Iter last);

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
    int ia[] = {0};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    Iter r = std::unique(Iter(ia), Iter(ia+sa));
    assert(base(r) == ia + sa);
    assert(ia[0] == 0);

    int ib[] = {0, 1};
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    r = std::unique(Iter(ib), Iter(ib+sb));
    assert(base(r) == ib + sb);
    assert(ib[0] == 0);
    assert(ib[1] == 1);

    int ic[] = {0, 0};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    r = std::unique(Iter(ic), Iter(ic+sc));
    assert(base(r) == ic + 1);
    assert(ic[0] == 0);

    int id[] = {0, 0, 1};
    const unsigned sd = sizeof(id)/sizeof(id[0]);
    r = std::unique(Iter(id), Iter(id+sd));
    assert(base(r) == id + 2);
    assert(id[0] == 0);
    assert(id[1] == 1);

    int ie[] = {0, 0, 1, 0};
    const unsigned se = sizeof(ie)/sizeof(ie[0]);
    r = std::unique(Iter(ie), Iter(ie+se));
    assert(base(r) == ie + 3);
    assert(ie[0] == 0);
    assert(ie[1] == 1);
    assert(ie[2] == 0);

    int ig[] = {0, 0, 1, 1};
    const unsigned sg = sizeof(ig)/sizeof(ig[0]);
    r = std::unique(Iter(ig), Iter(ig+sg));
    assert(base(r) == ig + 2);
    assert(ig[0] == 0);
    assert(ig[1] == 1);

    int ih[] = {0, 1, 1};
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    r = std::unique(Iter(ih), Iter(ih+sh));
    assert(base(r) == ih + 2);
    assert(ih[0] == 0);
    assert(ih[1] == 1);

    int ii[] = {0, 1, 1, 1, 2, 2, 2};
    const unsigned si = sizeof(ii)/sizeof(ii[0]);
    r = std::unique(Iter(ii), Iter(ii+si));
    assert(base(r) == ii + 3);
    assert(ii[0] == 0);
    assert(ii[1] == 1);
    assert(ii[2] == 2);
}

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

template <class Iter>
void
test1()
{
    MoveOnly ia[1] = {-1};
    const unsigned sa = sizeof(ia)/sizeof(ia[0]);
    Iter r = std::unique(Iter(ia), Iter(ia+sa));
    assert(base(r) == ia + sa);
    assert(ia[0] == -1);

    MoveOnly ib[2] = {-1, -1};
    ib[1].reset(1);
    const unsigned sb = sizeof(ib)/sizeof(ib[0]);
    r = std::unique(Iter(ib), Iter(ib+sb));
    assert(base(r) == ib + sb);
    assert(ib[0] == -1);
    assert(*ib[1] == 1);

    MoveOnly ic[2] = {-1, -1};
    const unsigned sc = sizeof(ic)/sizeof(ic[0]);
    r = std::unique(Iter(ic), Iter(ic+sc));
    assert(base(r) == ic + 1);
    assert(ic[0] == -1);

    MoveOnly id[3] = {-1, -1, -1};
    id[2].reset(1);
    const unsigned sd = sizeof(id)/sizeof(id[0]);
    r = std::unique(Iter(id), Iter(id+sd));
    assert(base(r) == id + 2);
    assert(id[0] == -1);
    assert(*id[1] == 1);

    MoveOnly ie[4] = {-1, -1, -1, -1};
    ie[2].reset(1);
    const unsigned se = sizeof(ie)/sizeof(ie[0]);
    r = std::unique(Iter(ie), Iter(ie+se));
    assert(base(r) == ie + 3);
    assert(ie[0] == -1);
    assert(*ie[1] == 1);
    assert(ie[2] == -1);

    MoveOnly ig[4] = {-1, -1, -1, -1};
    ig[2].reset(1);
    ig[3].reset(1);
    const unsigned sg = sizeof(ig)/sizeof(ig[0]);
    r = std::unique(Iter(ig), Iter(ig+sg));
    assert(base(r) == ig + 2);
    assert(ig[0] == -1);
    assert(*ig[1] == 1);

    MoveOnly ih[3] = {-1, -1, -1};
    ih[1].reset(1);
    ih[2].reset(1);
    const unsigned sh = sizeof(ih)/sizeof(ih[0]);
    r = std::unique(Iter(ih), Iter(ih+sh));
    assert(base(r) == ih + 2);
    assert(ih[0] == -1);
    assert(*ih[1] == 1);

    MoveOnly ii[7] = {-1, -1, -1, -1, -1, -1, -1};
    ii[1].reset(1);
    ii[2].reset(1);
    ii[3].reset(1);
    ii[4].reset(2);
    ii[5].reset(2);
    ii[6].reset(2);
    const unsigned si = sizeof(ii)/sizeof(ii[0]);
    r = std::unique(Iter(ii), Iter(ii+si));
    assert(base(r) == ii + 3);
    assert(ii[0] == -1);
    assert(*ii[1] == 1);
    assert(*ii[2] == 2);
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
