#pragma once

#include <algorithm>
#include <iterator>
#include <l.h>
#include <o.h>

template <class Y> L<Y> replicate(index_t n, Y y) {
    L<Y> r;
    r.reserve(n);
    std::fill_n(std::back_inserter(r), n, y);
    return r;
}

O take(O, O);
