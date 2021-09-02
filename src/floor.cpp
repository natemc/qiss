#include <floor.h>
#include <cmath>
#include <each.h>
#include <exception.h>
#include <flip.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    struct Floor {
        J operator()(B x) const { return J(B::rep(x)); }
        C operator()(C x) const { return C(C::rep(tolower(C::rep(x)))); }
        J operator()(F x) const { return J(J::rep(floor(F::rep(x)))); }
        J operator()(J x) const { return x; }

        O operator()(O x) const {
#define CS(X) case -OT<X>::typei(): return O((*this)(x.atom<X>()));      \
              case OT<X>::typei() : return each(*this, L<X>(std::move(x)))
            switch (int(x.type())) {
            CS(B); CS(C); CS(F); CS(J);
            case OT<O>::typei() : return each(*this, L<O>(std::move(x)));
            case '!': {
                auto [k, v] = UKV(std::move(x)).kv();
                return UKV(std::move(k), (*this)(std::move(v)));
            }
            case '+': return +(*this)(+std::move(x));
            default : throw Exception("type: _ (floor)");
            }
#undef CS
        }
    } floor__;
}

O floor_(O x) { return floor__(std::move(x)); }
