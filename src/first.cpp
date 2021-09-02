#include <first.h>
#include <object.h>
#include <each.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct First {
        template <class X> O operator()(L<X> x) const {
            if (x.empty()) throw Exception("length (*: on empty list)");
            return O(x[0]);
        }

        O operator()(O x) const {
#define CS(X) case OT<X>::typei(): return (*this)(L<X>(std::move(x)))
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
            CS(O);
            case '!': return (*this)(UKV(std::move(x)).val());
            case '+': {
                UKV d(addref(x->dict));
                return UKV(d.key(), each(*this, L<O>(d.val())));
            }
            default: throw Exception("nyi: *:");
            }
#undef CS
        }
    } first_;
} // unnamed

O first(O x) { return first_(x); }
