//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <size_t I, class... Types>
//   typename tuple_element<I, tuple<Types...> >::type&&
//   get(tuple<Types...>&& t);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include "MoveOnly.h"
#include <cassert>

int main()
{
    {
        typedef std::tuple<MoveOnly> T;
        T t(MoveOnly(3));
        MoveOnly p = std::get<0>(std::move(t));
        assert(*p == 3);
    }
}
