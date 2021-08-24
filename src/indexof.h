#pragma once

#include <algorithm>
#include <l.h>
#include <match.h>

const struct IndexOf {
    template <class X> index_t operator()(const L<X>& x, X y) const {
        if (x.is_sorted()) {
            const auto i = std::lower_bound(x.begin(), x.end(), y, match_) - x.begin();
            return i != x.size() && match_(x[i], y)? i : x.size();
        }
        // TODO exploit more attributes
        return std::find_if(x.begin(), x.end(),
            [&](const X& e){return match_(e, y);}) - x.begin();
    }
} indexof;
