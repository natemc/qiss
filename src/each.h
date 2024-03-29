#pragma once

#include <algorithm>
#include <enlist.h>
#include <exception.h>
#include <l.h>
#include <lambda.h>
#include <lcutil.h>
#include <o.h>
#include <object.h>
#include <objectio.h>
#include <primio.h>
#include <type_pair.h>
#include <type_traits>
#include <ukv.h>
#include <utility>

O each1     (ufun_t f, O x);
O each2     (bfun_t f, O x, O y);
O each_left (bfun_t f, O x, O y);
O each_right(bfun_t f, O x, O y);

const struct Each {
    template <class X> using OT = ObjectTraits<X>;

    template <class F, class X> requires std::is_invocable_v<F, X>
    O operator()(F&& f, L<X> x) const {
        using R = decltype(f(x[0]));
        if constexpr(std::is_same_v<R,O>)
            return object_ret(std::forward<F>(f), std::move(x));
        if constexpr(std::is_same_v<R,X>) {
            if (x.mine()) {
                std::transform(x.begin(), x.end(), x.begin(), std::forward<F>(f));
                return O(std::move(x));
            }
        }
        L<R> r(x.size());
        std::transform(x.begin(), x.end(), r.begin(), std::forward<F>(f));
        return O(std::move(r));
    }

    template <class F> requires std::is_invocable_v<F, O>
    O operator()(F&& f, L<O> x) const {
        using R = decltype(f(x[0]));
        if constexpr(std::is_same_v<R,O>)
            return object_ret(std::forward<F>(f), std::move(x));
        L<R> r(x.size());
        std::transform(x.begin(), x.end(), r.begin(), std::forward<F>(f));
        return O(std::move(r));
    }

    template <class FUN>
    O operator()(FUN&& f, O x) const {
        if (x.is_dict()) return (*this)(std::forward<FUN>(f), UKV(std::move(x)));
#define CS(X) case OT<X>::typei(): return (*this)(std::forward<FUN>(f), L<X>(std::move(x)))
        switch (int(x.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
        CS(S); CS(T); CS(X); CS(O);
        // TODO handle table
        default: throw Exception("nyi unary each");
        }
#undef CS
    }

    template <class F>
    O operator()(F&& f, UKV x) const {
        auto [k, v] = std::move(x).kv();
        return UKV(std::move(k), (*this)(std::forward<F>(f), std::move(v)));
    }

    template <class F, class X, class Y> requires std::is_invocable_v<F, X, Y>
    O operator()(F&& f, L<X> x, L<Y> y) const {
        if (x.size() != y.size()) throw Exception("length: each both");
        using R = decltype(f(x[0], y[0]));
        if constexpr(std::is_same_v<R,O>)
            return object_ret(std::forward<F>(f), std::move(x), std::move(y));
        if constexpr(std::is_same_v<R,X>) {
            if (x.mine()) {
                std::transform(x.begin(), x.end(), y.begin(), x.begin(),
                               std::forward<F>(f));
                return O(std::move(x));
            }
        }
        if constexpr(std::is_same_v<R,Y>) {
            if (y.mine()) {
                std::transform(x.begin(), x.end(), y.begin(), y.begin(),
                               std::forward<F>(f));
                return O(std::move(y));
            }
        }
        L<R> r(x.size());
        std::transform(x.begin(), x.end(), y.begin(), r.begin(),
                       std::forward<F>(f));
        return O(std::move(r));
    }

    auto operator()(ufun_t f, O x)      const { return each1(f, std::move(x)); }
    auto operator()(bfun_t f, O x, O y) const { return each2(f, std::move(x), std::move(y)); }

private:
    template <class F, class R, class X>
    O list_a(F&& f, R r0, R r1, L<X> x) const {
        // Even if R==X, we cannot reuse x, since we might need to abort.
        L<R> r(x.size());
        r[0] = r0;
        r[1] = r1;
        for (index_t i = 2; i < x.size(); ++i) {
            O rx(f(x[i]));
            if (rx.type() == -OT<R>::typet())
                r[i] = rx.atom<R>();
            else {
                L<O> p;
                p.reserve(x.size());
                for (index_t j = 0; j < i; ++j) p.emplace_back(r[j]);
                p.emplace_back(std::move(rx));
                for (; i < x.size(); ++i) p.emplace_back(f(x[i]));
                return O(std::move(p));
            }
        }
        return O(std::move(r));
    }

    template <class F, class R, class X, class Y>
    O list_a(F&& f, R r0, R r1, L<X> x, L<Y> y) const {
        assert(x.size() == y.size());
        // Even if R==X, we cannot reuse x, since we might need to abort.
        L<R> r(x.size());
        r[0] = r0;
        r[1] = r1;
        for (index_t i = 2; i < x.size(); ++i) {
            O rx(f(x[i], y[i]));
            if (rx.type() == -OT<R>::typet())
                r[i] = rx.atom<R>();
            else {
                L<O> p;
                p.reserve(x.size());
                for (index_t j = 0; j < i; ++j) p.emplace_back(r[j]);
                p.emplace_back(std::move(rx));
                for (; i < x.size(); ++i) p.emplace_back(f(x[i], y[i]));
                return O(std::move(p));
            }
        }
        return O(std::move(r));
    }

    template <class FUN, class Z>
    O object_ret(FUN&& f, L<Z> x) const {
        if (!x.size()) return L<O>{};
        O r0(f(x[0]));
        if (x.size() == 1) return enlist(std::move(r0));
        O r1(f(x[1]));
        if (!is_primitive_atom(r0.get()) || r0.type() != r1.type()) {
            L<O> r;
            r.reserve(x.size());
            r.emplace_back(std::move(r0));
            r.emplace_back(std::move(r1));
            std::transform(x.begin() + 2, x.end(), std::back_inserter(r), f);
            return O(std::move(r));
        }
#define CS(X) case -OT<X>::typei(): \
    return O(list_a(f, r0.atom<X>(), r1.atom<X>(), std::move(x)))
        switch (int(r0.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
        default: {
            L<C> s;
            s << "Unexpected type in each.h object_ret: " << r0.type() << '\n';
            throw lc2ex(s);
        }
        }
#undef CS
    }

    template <class FUN, class XX, class YY>
    O object_ret(FUN&& f, L<XX> x, L<YY> y) const {
        assert(x.size() == y.size());
        if (!x.size()) return L<O>{};
        O r0(f(x[0], y[0]));
        if (x.size() == 1) return enlist(std::move(r0));
        O r1(f(x[1], y[1]));
        if (!is_primitive_atom(r0.get()) || r0.type() != r1.type()) {
            L<O> r;
            r.reserve(x.size());
            r.emplace_back(std::move(r0));
            r.emplace_back(std::move(r1));
            std::transform(x.begin() + 2, x.end(), y.begin() + 2,
                           std::back_inserter(r), f);
            return O(std::move(r));
        }
#define CS(X) case -OT<X>::typei(): \
    return list_a(f, r0.atom<X>(), r1.atom<X>(), std::move(x), std::move(y))
        switch (int(r0.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
        default: {
            L<C> s;
            s << "Unexpected type in each.h object_ret: " << r0.type() << '\n';
            throw lc2ex(s);
        }
        }
#undef CS
    }
} each;

const struct EachBox1 {
    template <class X> using OT = ObjectTraits<X>;

    template <class F, class X> requires std::is_invocable_v<F, O>
    O operator()(F f, L<X> x) const {
        return each(L1(f(O(x))), std::move(x));
    }

    template <class F> requires std::is_invocable_v<F, O>
    O operator()(F&& f, Dict* x) const {
        return O(make_dict(addref(dk(x)),
                           (*this)(std::forward<F>(f), O(addref(dv(x)))).release()));
    }

    template <class FUN> requires std::is_invocable_v<FUN, O>
    O operator()(FUN&& f, O x) const {
        if (x.is_dict()) return (*this)(std::forward<FUN>(f), dict(x.get()));
#define CS(X) case OT<X>::typei(): return (*this)(std::forward<FUN>(f), L<X>(std::move(x)))
        switch (int(x.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
        CS(S); CS(T); CS(X);
        case OT<O>::typei():
            return each(std::forward<FUN>(f), L<O>(std::move(x)));
        // TODO handle dict, table
        default: throw Exception("nyi unary each");
        }
#undef CS
    }
} eachbox1;

const struct EachBox2 {
    template <class X> using OT = ObjectTraits<X>;

    template <class F, class X, class Y> requires std::is_invocable_v<F, O, O>
    O boxboth(F f, L<X> x, L<Y> y) const {
        return each(L2(f(O(x), O(y))), x, y);
    }

    template <class F, class X> requires std::is_invocable_v<F, O, O>
    O boxx(F f, L<X> x, L<O> y) const {
        return each(L2(f(O(x), std::move(y))), std::move(x), std::move(y));
    }

    template <class F, class Y> requires std::is_invocable_v<F, O, O>
    O boxy(F f, L<O> x, L<Y> y) const {
        return each(L2(f(std::move(x), O(y))), std::move(x), std::move(y));
    }

    template <class FUN> requires std::is_invocable_v<FUN, O, O>
    O operator()(FUN f, O x, O y) const {
        if      (x.is_atom())
            return y.is_atom()? f(std::move(x), std::move(y))
                :  /* else */   eachbox1([&](auto e){ return f(e, y); }, std::move(x));
        else if (y.is_atom()) return eachbox1([&](auto e){ return f(x, e); }, std::move(y));
#define BB(X,Y)                                                                        \
    case TypePair<X,Y>::LL: return boxboth(f, L<X>(std::move(x)), L<Y>(std::move(y)))
#define BO(X)                                                                          \
    case TypePair<X,O>::LL: return boxx   (f, L<X>(std::move(x)), L<O>(std::move(y))); \
    case TypePair<O,X>::LL: return boxy   (f, L<O>(std::move(x)), L<X>(std::move(y)))
#define AA(Z) BB(Z,B); BB(Z,C); BB(Z,D); BB(Z,F); BB(Z,H); BB(Z,I); \
              BB(Z,J); BB(Z,S); BB(Z,T); BB(Z,X)
        switch (type_pair(x, y)) {
        AA(B); AA(C); AA(D); AA(F); AA(H); AA(I); AA(J);
        AA(S); AA(T); AA(X);
        BO(B); BO(C); BO(D); BO(F); BO(H); BO(I); BO(J);
        BO(S); BO(T); BO(X);
        case TypePair<O,O>::LL:
            return each(f, L<O>(std::move(x)), L<O>(std::move(y)));
        // TODO handle dict, table
        default: throw Exception("nyi binary each");
        }
#undef AA
#undef BB
#undef BO
    }
} eachbox2;

const struct EachL {
    template <class F, class X, class Y> requires std::is_invocable_v<F, X, Y>
    auto operator()(F f, L<X> x, const Y& y) const {
        return each([&](auto e){ return f(e, y); }, std::move(x));
    }

    auto operator()(bfun_t f, O x, O y) const {
        return each_left(f, std::move(x), std::move(y));
    }
} eachL;

const struct EachR {
    template <class F, class X, class Y> requires std::is_invocable_v<F, X, Y>
    auto operator()(F f, const X& x, L<Y> y) const {
        return each([&](auto e){ return f(x, e); }, std::move(y));
    }

    auto operator()(bfun_t f, O x, O y) const {
        return each_right(f, std::move(x), std::move(y));
    }
} eachR;
