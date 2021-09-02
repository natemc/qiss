#include <reverse.h>
#include <algorithm>
#include <each.h>
#include <exception.h>
#include <o.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Reverse {
        template <class X> L<X> operator()(L<X> x) const {
            if (x.mine()) {
                std::reverse(x.begin(), x.end());
                return x;
            }
            L<X> r(x.size());
            std::reverse_copy(x.begin(), x.end(), r.begin());
            return r;
        }

        O operator()(O x) const {
#define CS(X) case OT<X>::typei(): return (*this)(L<X>(std::move(x)))
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X); CS(O);
            case '!': {
                auto [k, v] = UKV(std::move(x)).kv();
                return UKV((*this)(std::move(k)), (*this)(std::move(v)));
            }
            case '+': {
                UKV d(addref(x->dict));
                return O(make_table(d.key().release(),
                                    each(*this, L<O>(d.val())).release()));
            }
            default: throw Exception("nyi: |:");
            }
#undef CS
        }
    } reverse_;
} // unnamed

O reverse(O x) { return reverse_(x); }
