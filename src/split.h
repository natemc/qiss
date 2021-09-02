#pragma once

#include <algorithm>
#include <l.h>

template <class X> L<L<X>> split(X delim, const L<X>& s) {
    L<L<X>> r;
    r.reserve(std::count(s.begin(), s.end(), delim) + 1);
    auto p = s.begin();
    auto q = std::find(p, s.end(), delim);
    for (; q != s.end(); p = q + 1, q = std::find(p, s.end(), delim))
        r.emplace_back(p, q);
    r.emplace_back(p, q);
    return r;
}
