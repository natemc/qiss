#pragma once

#include <exception.h>
#include <l.h>
#include <o.h>
#include <object.h>
#include <prim.h>
#include <type_traits>
#include <ukv.h>
#include <utility>

O over1(bfun_t f, O x);
O over2(bfun_t f, O x, O y);

const struct Over {
    template <class X> using OT = ObjectTraits<X>;
    
    template <class F, class Y> requires std::is_invocable_r_v<O, F, O, O>
    O operator()(F f, O x, const Y* first, const Y* last) const {
        if (first == last) return x;
        O r(f(std::move(x), O(*first)));
        for (++first; first != last; ++first) r = f(std::move(r), O(*first));
        return r;
    }

    template <class F> requires std::is_invocable_r_v<O, F, O, O>
    O operator()(F f, O x, O* first, O* last) const {
        if (first == last) return x;
        O r(f(std::move(x), *first));
        for (++first; first != last; ++first) r = f(std::move(r), *first);
        return r;
    }

    
    template <class F, class Y> requires std::is_invocable_r_v<O, F, O, O>
    O operator()(F&& f, O x, L<Y> y) const {
        return (*this)(std::forward<F>(f), std::move(x), y.begin(), y.end());
    }

    template <class F, class Y> requires std::is_invocable_r_v<O, F, O, O>
    O operator()(F&& f, L<Y> y) const {
        if (y.empty()) throw Exception("length: unary over on empty list");
        return (*this)(std::forward<F>(f), O(y[0]), y.begin() + 1, y.end());
    }

    template <class F> requires std::is_invocable_r_v<O, F, O, O>
    O operator()(F&& f, L<O> y) const {
        if (y.empty()) throw Exception("length: unary over on empty list");
        return (*this)(std::forward<F>(f), y[0], y.begin() + 1, y.end());
    }

    template <class FUN> O operator()(FUN&& f, O y) const {
#define CS(Y) case OT<Y>::typei(): return (*this)(std::forward<FUN>(f), L<Y>(std::move(y)))
        switch (int(y->type)) {
        CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X); CS(O);
        case '!': return (*this)(f, UKV(std::move(y)).val());
        default: throw Exception("type: over's rhs must be list or dict");
        }
#undef CS
    }

    template <class FUN> O operator()(FUN&& f, O x, O y) const {
#define CS(Y) case OT<Y>::typei():                                       \
    return (*this)(std::forward<FUN>(f), std::move(x), L<Y>(std::move(y)))
        switch (int(y->type)) {
        CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X); CS(O);
        case '!': return (*this)(f, std::move(x), UKV(std::move(y)).val());
        default: throw Exception("type: over's rhs must be list or dict");
        }
#undef CS
    }
} over;
