#include <null.h>
#include <each.h>
#include <flip.h>
#include <l.h>
#include <o.h>
#include <primio.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Null {
        template <class X> B operator()(X x) const { return B(x.is_null()); }

        O operator()(O x) const {
#define CS(X) case -OT<X>::typei(): return O((*this)(x.atom<X>()));      \
              case OT<X>::typei() : return each(*this, L<X>(std::move(x)))
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
            case OT<O>::typei() : return each(*this, L<O>(std::move(x)));
            case '!': {
                auto [k, v] = UKV(std::move(x)).kv();
                return UKV(std::move(k), (*this)(std::move(v)));
            }
            case '+': return +(*this)(+std::move(x));
            default : throw Exception("type: _ (null)");
            }
#undef CS
        }
    } null_;
}

O null(O x) { return null_(x); }
