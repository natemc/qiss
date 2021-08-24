#pragma once

#include <algorithm>
#include <l.h>
#include <o.h>

O match(O x, O y);

const struct Match_ {
    bool operator()(O x, O y) const;
    template <class X> bool operator()(L<X> x, L<X> y) const {
        return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin(), *this);
    }
#define EQ(X) bool operator()(X x, X y) const { return x == y; }
    EQ(B) EQ(C) EQ(D) EQ(F) EQ(H) EQ(I) EQ(J) EQ(S) EQ(T) EQ(X)
#undef EQ
} match_;
