#include <fill.h>
#include <each.h>
#include <exception.h>
#include <flip.h>
#include <l.h>
#include <o.h>
#include <type_pair.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class X> struct Fill {
        explicit Fill(X x_): x(x_) {}
        X operator()(X y) const { return y.is_null()? x : y; }

        O operator()(O y) const {
            switch (int(y.type())) {
            case -OT<X>::typei(): return O((*this)(y.atom<X>()));
            case OT<X>::typei() : return each(*this, L<X>(std::move(y)));
            case OT<O>::typei() : return each(*this, L<O>(std::move(y)));
            case '!': {
                auto [k, v] = UKV(std::move(y)).kv();
                return UKV(std::move(k), (*this)(std::move(v)));
            }
            case '+': return flip((*this)(flip(std::move(y))));
            default : throw Exception("type: ^ (fill)");
            }
#undef CS
        }
    private:
        X x;
    };
  
    template <class X> O fill_(X x, O y) { return Fill<X>(x)(std::move(y)); }
}

O fill(O x, O y) {
    if (type_pair(x, y) == TypePair<I,F>::AA || type_pair(x, y) == TypePair<I,F>::AL)
        return fill_(F(I::rep(x.atom<I>())), std::move(y));
    if (type_pair(x, y) == TypePair<J,F>::AA || type_pair(x, y) == TypePair<J,F>::AL)
        return fill_(F(F::rep(J::rep(x.atom<J>()))), std::move(y));
#define CS(X) case -OT<X>::typei(): return fill_(x.atom<X>(), std::move(y))
    switch (int(x.type())) {
    CS(B); CS(C); CS(D); CS(F); CS(I); CS(J); CS(S); CS(T); CS(X);
    default : throw Exception("type ^ (fill)");
    }
#undef CS
}
