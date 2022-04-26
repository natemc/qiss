#pragma once

#include <algorithm>
#include <exception.h>
#include <indexof.h>
#include <l.h>
#include <lambda.h>
#include <o.h>
#include <type_traits>
#include <ukv.h>
#include <utility>

const struct MergeDicts {
    template <class X> using OT = ObjectTraits<X>;

    template <class F, class K, class Y> requires is_prim_v<Y>
    UKV operator()(F f, L<K> xk, L<O> xv, L<K> yk, L<Y> yv) const {
        return merge_(L2(f(x, O(y))), std::move(xk), std::move(xv), std::move(yk), std::move(yv));
    }

    template <class F, class K, class X> requires is_prim_v<X>
    UKV operator()(F f, L<K> xk, L<X> xv, L<K> yk, L<O> yv) const {
        return merge_(L2(f(O(x), y)), std::move(xk), std::move(xv), std::move(yk), std::move(yv));
    }

    template <class F, class K, class X, class Y>
    UKV operator()(F&& f, L<K> xk, L<X> xv, L<K> yk, L<Y> yv) const {
        return merge_(std::forward<F>(f), std::move(xk), std::move(xv), std::move(yk), std::move(yv));
    }

    template <class F, class K, class X, class Y>
    UKV merge_(F f, L<K> xk, L<X> xv, L<K> yk, L<Y> yv) const {
        // TODO use a grouper
        using V = decltype(f(xv[0], yv[0]));
        L<V> rv;
        L<K> rk   (xk.begin(), xk.end());
        L<B> yused(yk.size()); std::fill(yused.begin(), yused.end(), B(false));
        for (index_t xi = 0; xi < xk.size(); ++xi) {
            const index_t yi = indexof(yk, xk[xi]);
            if (yi != yk.size()) {
                yused[yi] = B(true);
                rv.emplace_back(f(xv[xi], yv[yi]));
            } else {
                if constexpr(std::is_constructible_v<V, X>)
                    rv.emplace_back(xv[xi]);
                else if constexpr(std::is_constructible_v<V, typename X::rep>)
                    rv.emplace_back(typename V::rep(typename X::rep(xv[xi])));
                else
                    throw Exception("type: merge dicts (no match in lhs)");
            }
        }
        for (index_t i = 0; i < yk.size(); ++i) {
            if (!yused[i]) {
                rk.emplace_back(yk[i]);
                if constexpr(std::is_constructible_v<V, Y>)
                    rv.emplace_back(yv[i]);
                else if constexpr(std::is_constructible_v<V, typename Y::rep>)
                    rv.emplace_back(typename V::rep(typename Y::rep(yv[i])));
                else
                    throw Exception("type: merge dicts (no match in rhs)");
            }
        }
        return UKV(rk, rv);
    }

    template <class FUN, class XX, class YY>
    UKV operator()(FUN&& f, O xk, L<XX> xv, O yk, L<YY> yv) const {
        if (xk.type() != yk.type())
            throw Exception("type: cannot merge 2 dicts w/different key types");
#define CS(Z) case OT<Z>::typei(): return (*this)( \
    std::forward<FUN>(f), L<Z>(std::move(xk)), std::move(xv), L<Z>(std::move(yk)), std::move(yv))
        switch (int(xk.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(J); CS(S); CS(T); CS(X); CS(O);
        default: throw Exception("nyi op on 2 dicts - keyed tables??");
        }
#undef CS
    }
} merge_dicts;
