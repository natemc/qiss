#pragma once

#include <type_traits>
#include <utility>

template <class X, class F> struct BoundLhs {
    BoundLhs(X x_, F f_): x(x_), f(f_) {}

    template <class Y> requires std::is_invocable_v<F, X, Y>
    decltype(auto) operator^(Y&& y) && {
        return std::move(f)(std::move(x), std::forward<Y>(y));
    }

 private:
    X x;
    F f;
};

template <class X, class F> BoundLhs<X,F>
operator^(X&& x, F&& f) { return BoundLhs<X,F>(std::forward<X>(x), std::forward<F>(f)); }
