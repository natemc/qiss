#pragma once

#include <arith.h>
#include <l.h>
#include <not.h>
#include <o.h>
#include <object.h>
#include <prim.h>
#include <relop.h>
#include <utility>

template <class X> requires is_prim_v<X> L<B> operator==(L<X> x, X y) {
    Object b(box(y));
    return L<B>(eq(std::move(x), O(&b)));
}

template <class X> requires is_prim_v<X> L<B> operator==(X x, L<X> y) {
    return std::move(y) == x;
}

template <class X> requires is_prim_v<X>
L<B> operator==(L<X> x, typename X::rep y) { return std::move(x) == X(y); }

template <class X> requires is_prim_v<X>
L<B> operator==(typename X::rep x, L<X> y) { return std::move(y) == X(x); }

template <class X, class Y> L<B> operator==(L<X> x, L<Y> y) {
    return L<B>(eq(std::move(x), std::move(y)));
}

template <class X> requires is_prim_v<X> L<B> operator!=(L<X> x, X y) {
    Object b(box(y));
    return L<B>(not_(eq(std::move(x), O(&b))));
}

template <class X> requires is_prim_v<X> L<B> operator!=(X x, L<X> y) {
    return std::move(y) != x;
}

template <class X> requires is_prim_v<X>
L<B> operator!=(L<X> x, typename X::rep y) { return std::move(x) != X(y); }

template <class X> requires is_prim_v<X>
L<B> operator!=(typename X::rep x, L<X> y) { return std::move(y) != X(x); }

template <class X, class Y> L<B> operator!=(L<X> x, L<Y> y) {
    return L<B>(not_(eq(std::move(x), std::move(y))));
}



template <class X> requires is_prim_v<X> L<X> operator+(L<X> x, X y) {
    Object b(box(y));
    return L<X>(add(std::move(x), O(&b)));
}

template <class X> requires is_prim_v<X>
L<X> operator+(X x, L<X> y) { return std::move(y) + x; }

template <class X> requires is_prim_v<X>
L<X> operator+(L<X> x, typename X::rep y) { return std::move(x) + X(y); }

template <class X> requires is_prim_v<X>
L<X> operator+(typename X::rep x, L<X> y) { return std::move(y) + X(x); }

template <class X, class Y> auto operator+(L<X> x, L<Y> y) {
    return add(std::move(x), std::move(y));
}

template <class X> requires is_prim_v<X> L<X> operator-(L<X> x, X y) {
    Object b(box(y));
    return L<X>(sub(std::move(x), O(&b)));
}

template <class X> requires is_prim_v<X> L<X> operator-(X x, L<X> y) {
    Object b(box(x));
    return sub(O(&b), std::move(y));
}

template <class X> requires is_prim_v<X>
L<X> operator-(L<X> x, typename X::rep y) { return std::move(x) - X(y); }

template <class X> requires is_prim_v<X>
L<X> operator-(typename X::rep x, L<X> y) { return X(x) - std::move(y); }

template <class X, class Y> auto operator-(L<X> x, L<Y> y) {
    return sub(std::move(x), std::move(y));
}
