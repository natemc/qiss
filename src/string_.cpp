#include <string_.h>
#include <each.h>
#include <flip.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct String_ {
        template <class X> L<C> operator()(X x) const {
            L<C> s;
            s << x;
            return s;
        }

        UKV operator()(UKV x) const {
            auto [k, v] = std::move(x).kv();
            return UKV(std::move(k), (*this)(std::move(v)));
        }

        O operator()(O x) const {
#define CS(X) case -OT<X>::typei(): return (*this)(x.atom<X>()); \
              case OT<X>::typei() : return each(*this, L<X>(std::move(x)))
            switch(int(x->type)) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X);
            case OT<O>::typei(): return each(*this, L<O>(std::move(x)));
            case '!': return (*this)(UKV(std::move(x)));
            case '+': return +(*this)(flip(std::move(x)));
            default : throw Exception("nyi: $ (string) on unknown type");
            }
#undef CS
        }
    } string__;
}

O string_(O x) { return string__(std::move(x)); }
