#include <distinct.h>
#include <algorithm>
#include <flip.h>
#include <hashset.h>
#include <limits>
#include <o.h>
#include <objectio.h>
#include <primio.h>
#include <sum.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class X> L<X> hash_distinct(L<X> x) {
        L<X> r;
        HashSet<X> s(x);
        for (index_t i = 0; i < x.size(); ++i) {
            if (s.insert(i)) r.push_back(x[i]);
        }
        return r;
    }

    template <class X> L<X> integral_distinct(L<X> x) {
        return hash_distinct(std::move(x));
        /*
        // TODO test if/when this is faster
        constexpr index_t threshold = 65536;
        using R = typename X::rep;
        auto [lo, hi] = std::minmax_element(x.begin(), x.end());
        if (threshold < R(*hi) - R(*lo)) return hash_distinct(std::move(x));
        uint8_t counts[threshold] = {};
        for (index_t i = 0; i < x.size(); ++i) counts[R(x[i]) - R(*lo)] = 1;
        L<X> r(sum(counts));
        index_t k = 0;
        for (index_t i = 0; i < x.size(); ++i) {
            const index_t j = R(x[i]) - R(*lo);
            if (counts[j]) {
                counts[j] = 0;
                r[k++] = x[i];
            }
        }
        return r;
        */
    }

    const struct Distinct {
        template <class X> L<X> operator()(L<X> x) const {
            return hash_distinct(std::move(x));
        }

        L<B> operator()(L<B> x) const {
            if (x.size() < 2) return x;
            if (x.mine()) {
                const auto it = std::find(
                    x.begin() + 1, x.end(), B(1 - B::rep(x[0])));
                x.trunc(it == x.end()? 1 : 2);
                return x;
            }
            L<B> r(2);
            r[0] = x[0];
            const auto it = std::find(x.begin() + 1, x.end(), B(1 - B::rep(x[0])));
            if (it == x.end()) r.trunc(1);
            else               r[1] = *it;
            return r;
        }

        L<C> operator()(L<C> x) const { return integral_distinct(x); }
        L<D> operator()(L<D> x) const { return integral_distinct(x); }
        L<H> operator()(L<H> x) const { return integral_distinct(x); }
        L<I> operator()(L<I> x) const { return integral_distinct(x); }
        L<J> operator()(L<J> x) const { return integral_distinct(x); }
        L<S> operator()(L<S> x) const { return integral_distinct(x); }
        L<T> operator()(L<T> x) const { return integral_distinct(x); }
        L<X> operator()(L<X> x) const { return integral_distinct(x); }

        O operator()(O x) const {
#define CS(X) case OT<X>::typei(): return (*this)(L<X>(std::move(x)))
            switch (int(x->type)) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
            case OT<O>::typei(): return hash_distinct(L<O>(std::move(x)));
            case '!': return (*this)(UKV(std::move(x)).val());
            // TODO research if there's a better way than two flips :-/
            case '+': {
                auto [k, v] = UKV(+std::move(x)).kv();
                return +UKV(std::move(k), +hash_distinct(L<O>(+std::move(v))));
            }
            default: throw Exception("nyi: ?:");
            }
#undef CS
        }
    } distinct_;
} // unnamed

O distinct(O x) { return distinct_(x); }
