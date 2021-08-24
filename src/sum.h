#pragma once

#include <l.h>

const struct Sum {
    template <class X> auto operator()(const X* first, const X* last) const {
        using R = decltype(*first + *first);
        R r(0);
        for (; first != last; ++first) r += *first;
        return r;
    }

    template <class X> auto operator()(const L<X>& x) const {
        return (*this)(x.begin(), x.end());
    }

    template <class X, std::size_t N>
    auto operator()(const X(&a)[N]) const { return (*this)(a, a + N); }
} sum;
