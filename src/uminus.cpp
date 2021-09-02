#include <uminus.h>
#include <each.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct UMinus {
        J operator()(B x) const { return J(-J::rep(B::rep(x))); }
        F operator()(F x) const { return F(-F::rep(x)); }
        I operator()(I x) const { return I(-I::rep(x)); }
        J operator()(J x) const { return J(-J::rep(x)); }
        O operator()(O x) const {
#define CS(X) case -OT<X>::typei(): return O((*this)(x.atom<X>()));      \
              case OT<X>::typei() : return each(*this, L<X>(std::move(x)))
            switch (int(x.type())) {
            CS(B); CS(F); CS(I); CS(J);
            case OT<O>::typei(): return each(*this, L<O>(std::move(x)));
            case '!': {
                auto [k, v] = UKV(std::move(x)).kv();
                return UKV(k, (*this)(std::move(v)));
            }
            case '+': {
                UKV d(addref(x->dict));
                return O(make_table(d.key().release(),
                                    (*this)(d.val()).release()));
            }
            default: throw Exception("type (unary -)");
        }
        }
    } uminus_;
}

O uminus(O x) { return uminus_(std::move(x)); }
