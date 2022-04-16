#include <at.h>
#include <algorithm>
#include <each.h>
#include <find.h>
#include <flip.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <objectio.h>
#include <type_pair.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;
    template <class X, class Y> using TP = TypePair<X, Y>;

    const struct At {
        template <class X>
        X operator()(const L<X>& x, index_t y) const {
            return 0 <= y && y < x.size()? x[y] : OT<X>::null();
        }

        template <class X>
        X operator()(const L<X>& x, B y) const { return (*this)(x, B::rep(y)); }
        template <class X>
        X operator()(const L<X>& x, I y) const { return (*this)(x, I::rep(y)); }
        template <class X>
        X operator()(const L<X>& x, J y) const { return (*this)(x, J::rep(y)); }

        template <class X, class Y>
        auto operator()(const L<X>& x, L<Y> y) const {
            return eachR(*this, x, std::move(y));
        }

        O operator()(O x, O y) const {
#define CS(X)                                                               \
case TP<X,B>::LA: return O((*this)(L<X>(std::move(x)), y.atom<B>()));       \
case TP<X,B>::LL: return   (*this)(L<X>(std::move(x)), L<B>(std::move(y))); \
case TP<X,I>::LA: return O((*this)(L<X>(std::move(x)), y.atom<I>()));       \
case TP<X,I>::LL: return   (*this)(L<X>(std::move(x)), L<I>(std::move(y))); \
case TP<X,J>::LA: return O((*this)(L<X>(std::move(x)), y.atom<J>()));       \
case TP<X,J>::LL: return   (*this)(L<X>(std::move(x)), L<J>(std::move(y))); \
case TP<X,O>::LL: return   (*this)(L<X>(std::move(x)), L<O>(std::move(y)))
            switch (type_pair(x, y)) {
            CS(B); CS(C); CS(D); CS(F); CS(I); CS(J); CS(S); CS(T); CS(X);
            case TP<O,I>::LA: return (*this)(L<O>(std::move(x)), y.atom<I>());
            case TP<O,I>::LL: return (*this)(L<O>(std::move(x)), L<I>(std::move(y)));
            case TP<O,J>::LA: return (*this)(L<O>(std::move(x)), y.atom<J>());
            case TP<O,J>::LL: return (*this)(L<O>(std::move(x)), L<J>(std::move(y)));
            case TP<O,O>::LL: return (*this)(L<O>(std::move(x)), L<O>(std::move(y)));
            default: throw Exception("nyi: @");
            }
#undef CS
        }
    } at_;

    O at_dict(UKV x, O   y) { return at(x.val(), find(x.key(), std::move(y))); }
    O at_dict(O   x, UKV y) {
        auto [k, v] = std::move(y).kv();
        return UKV(std::move(k), at(std::move(x), std::move(v)));
    }

    O row(O x, O y) {
        auto [k, v] = UKV(+std::move(x)).kv();
        return UKV(std::move(k),
                   each([=](O e){ return at_(std::move(e), y); },
                        L<O>(std::move(v))));
    }

    template <class X> O rows(O x, L<X> y) {
        return +row(std::move(x), std::move(y));
    }

    O at_table(O x, O y) {
        switch (int(y.type())) {
        case -OT<S>::typei():
        case OT<S>::typei() : return at_dict(UKV(+std::move(x)), std::move(y));
        case -OT<I>::typei(): return row (std::move(x), std::move(y));
        case -OT<J>::typei(): return row (std::move(x), std::move(y));
        case OT<I>::typei() : return rows(std::move(x), L<I>(std::move(y)));
        case OT<J>::typei() : return rows(std::move(x), L<J>(std::move(y)));
        default             : {
            L<C> s;
            s << "type (@): can't index table with " << y.type();
            throw lc2ex(s);
            }
        }
    }
} // unnamed

O at(O x, O y) {
    return x.is_dict ()? at_dict (UKV(std::move(x)), std::move(y))
         : y.is_dict ()? at_dict (std::move(x)     , UKV(std::move(y)))
         : x.is_table()? at_table(std::move(x)     , std::move(y))
         : /* else */    at_     (std::move(x)     , std::move(y));
}
