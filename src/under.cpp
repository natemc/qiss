#include <under.h>
#include <algorithm>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Cut {
        template <class X, class Y> requires is_prim_v<X>
        O operator()(L<X> x, L<Y> y) const {
            using R = typename X::rep;
            L<L<Y>> r;
            r.reserve(x.size());
            for (index_t i = 0; i < x.size() - 1; ++i)
                r.emplace_back(y.begin() + R(x[i]), y.begin() + R(x[i + 1]));
            r.emplace_back(y.begin() + R(x.back()), y.end());
            return O(std::move(r));
        }

        template <class Z> O operator()(L<Z> x, O y) const {
#define CS(Y) case OT<Y>::typei(): return (*this)(std::move(x), L<Y>(std::move(y)))
            switch (int(y.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X); CS(O);
            case '!': {
                auto [k, v] = UKV(std::move(y)).kv();
                return UKV((*this)(x, std::move(k)), (*this)(x, std::move(v)));
            }
            case '+': throw Exception("nyi: _ (cut) on table");
            default : throw Exception("nyi: _ (cut) on unknown type");
            }
#undef CS
        }
    } cut;

    const struct Drop {
        UKV operator()(L<S>, UKV) const {
            throw Exception("nyi: _ (drop) from dict via sym");
        }

        template <class Y> L<Y> operator()(index_t x, L<Y> y) const {
            return x<0? L<Y>(y.begin(), y.end() - std::min(y.size(), -x))
                :       L<Y>(y.begin() + std::min(y.size(), x), y.end());
        }

        O operator()(index_t x, O y) const {
#define CS(Y) case OT<Y>::typei(): return (*this)(std::move(x), L<Y>(std::move(y)))
            switch (int(y.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X); CS(O);
            case '!': {
                auto [k, v] = UKV(std::move(y)).kv();
                return UKV((*this)(x, std::move(k)), (*this)(x, std::move(v)));
            }
            case '+': throw Exception("nyi: _ (drop) on table");
            default : throw Exception("nyi: _ (drop) on unknown type");
            }
#undef CS
        }
    } drop;

    const struct Remove {
        template <class X> auto operator()(L<X> x, index_t y) const {
            if (y < 0 || x.size() <= y) return x;
            L<X> r(x.size() - 1);
            std::copy(x.begin(), x.begin() + y, r.begin());
            std::copy(x.begin() + y + 1, x.end(), r.begin() + y);
            return r;
        }

        O operator()(O x, index_t y) const {
#define CS(X) case OT<X>::typei(): return (*this)(L<X>(std::move(x)), y)
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X); CS(O);
            default: throw Exception("nyi: _");
            }
#undef CS
        }
    } remove_;
} // unnamed

O under(O x, O y) {
    if (x.type() == OT<S>::typet()  && y.is_dict())
        return drop(L<S>(std::move(x)), UKV(std::move(y)));
    if (x.type() == -OT<S>::typet() && y.is_dict())
        return drop(L<S>{x.atom<S>()}, UKV(std::move(y)));
    if (y.type() == -OT<B>::typet()) return remove_(std::move(x), B::rep(y.atom<B>()));
    if (y.type() == -OT<I>::typet()) return remove_(std::move(x), I::rep(y.atom<I>()));
    if (y.type() == -OT<J>::typet()) return remove_(std::move(x), J::rep(y.atom<J>()));
    switch (int(x.type())) {
    case -OT<B>::typei(): return drop(B::rep(x.atom<B>()), std::move(y));
    case -OT<I>::typei(): return drop(I::rep(x.atom<I>()), std::move(y));
    case -OT<J>::typei(): return drop(J::rep(x.atom<J>()), std::move(y));
    case OT<B>::typei() : return cut (L<B>(std::move(x)) , std::move(y));
    case OT<I>::typei() : return cut (L<I>(std::move(x)) , std::move(y));
    case OT<J>::typei() : return cut (L<J>(std::move(x)) , std::move(y));
    default             : throw Exception("type: _");
    }
}
