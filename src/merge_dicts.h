#pragma once

#include <algorithm>
#include <exception.h>
#include <indexof.h>
#include <l.h>
#include <o.h>
#include <type_traits>
#include <ukv.h>

const struct MergeDicts {
    template <class X> using OT = ObjectTraits<X>;

    template <class F, class K, class X, class Y>
    UKV merge(F f, L<K> xk, L<X> xv, L<K> yk, L<Y> yv) const {
        // TODO use a grouper
        using V = decltype(f(xv[0], yv[0]));
        L<B> yused(yk.size()); std::fill(yused.begin(), yused.end(), B(false));
        L<K> rk   (xk.begin(), xk.end());
        L<V> rv;
        for (index_t xi = 0; xi < xk.size(); ++xi) {
            const index_t yi = indexof(yk, xk[xi]);
            if (yi == yk.size()) {
                if constexpr(!is_prim_v<X>)
                    rv.emplace_back(xv[xi]);
                else if constexpr(std::is_constructible_v<V, typename X::rep>)
                    rv.emplace_back(typename V::rep(typename X::rep(xv[xi])));
                else
                    throw Exception("type: merge dicts");
            } else {
                yused[yi] = B(true);
                rv.emplace_back(f(xv[xi], yv[yi]));
            }
        }
        for (index_t i = 0; i < yk.size(); ++i) {
            if (!yused[i]) {
                rk.emplace_back(yk[i]);
                if constexpr(!is_prim_v<Y>)
                    rv.emplace_back(yv[i]);
                else if constexpr(std::is_constructible_v<V, typename Y::rep>)
                    rv.emplace_back(typename V::rep(typename Y::rep(yv[i])));
                else
                    throw Exception("type: merge dicts");
            }
        }
        return UKV(rk, rv);
    }

    template <class FUN, class XX, class YY>
    UKV operator()(FUN&& f, O xk, L<XX> xv, O yk, L<YY> yv) const {
        if (xk.type() != yk.type())
            throw Exception("type: cannot merge 2 dicts w/different key types");
#define CS(X) case OT<X>::typei(): return merge( \
    std::forward<FUN>(f), L<X>(std::move(xk)), std::move(xv), L<X>(std::move(yk)), std::move(yv))
        switch (int(xk.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(J); CS(S); CS(T); CS(X); CS(O);
        default: throw Exception("nyi op on 2 dicts - keyed tables??");
        }
#undef CS
    }
} merge_dicts;
