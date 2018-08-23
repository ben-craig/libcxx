#ifndef POINTER_COMPARISON_TEST_HELPER_HPP
#define POINTER_COMPARISON_TEST_HELPER_HPP

#include <array>
#include <memory>
#include <cstdint>
#include <cassert>

#include "test_macros.h"

template <class T, template<class> class CompareTemplate>
void do_pointer_comparison_test() {
    typedef CompareTemplate<T*> Compare;
    typedef CompareTemplate<std::uintptr_t> UIntCompare;
#if TEST_STD_VER > 11
    typedef CompareTemplate<void> VoidCompare;
#else
    typedef Compare VoidCompare;
#endif
    const std::size_t test_size = 100;
    std::array<T, test_size> pointers;
    for (size_t i=0; i < test_size; ++i)
        pointers[i] = T();
    Compare comp;
    UIntCompare ucomp;
    VoidCompare vcomp;
    for (size_t i=0; i < test_size; ++i) {
        for (size_t j=0; j < test_size; ++j) {
            T* lhs = &pointers[i];
            T* rhs = &pointers[j];
            std::uintptr_t lhs_uint = reinterpret_cast<std::uintptr_t>(lhs);
            std::uintptr_t rhs_uint = reinterpret_cast<std::uintptr_t>(rhs);
            assert(comp(lhs, rhs) == ucomp(lhs_uint, rhs_uint));
            assert(vcomp(lhs, rhs) == ucomp(lhs_uint, rhs_uint));
        }
    }
}

#endif // POINTER_COMPARISON_TEST_HELPER_HPP
