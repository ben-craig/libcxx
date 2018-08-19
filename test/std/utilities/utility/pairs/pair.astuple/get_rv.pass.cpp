//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// template<size_t I, class T1, class T2>
//     typename tuple_element<I, std::pair<T1, T2> >::type&&
//     get(pair<T1, T2>&&);

#include <utility>
#include <memory>
#include <cassert>

int main()
{
    {
        int value = 3;
        auto nop_deleter = [](auto *){};
        typedef std::unique_ptr<int, decltype(nop_deleter)> my_ptr;
        typedef std::pair<my_ptr, short> P;
        P p(my_ptr(&value, nop_deleter), static_cast<short>(4));
        my_ptr ptr = std::get<0>(std::move(p));
        assert(*ptr == 3);
    }
}
