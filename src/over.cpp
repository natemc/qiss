#include <over.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Over {
        template <class Y>
        O operator()(bfun_t f, O x, Y* first, Y* last) const {
            if (first == last) return x;
            Object boxy(box(*first));
            O r(f(std::move(x), O(&boxy)));
            for (++first; first != last; ++first) {
                Object b(box(*first));
                r = f(std::move(r), O(&b));
            }
            return r;
        }

        O operator()(bfun_t f, O x, O* first, O* last) const {
            if (first == last) return x;
            O r(f(std::move(x), *first));
            for (++first; first != last; ++first) r = f(std::move(r), *first);
            return r;
        }

        template <class Y>
        O operator()(bfun_t f, O x, L<Y> y) const {
            return (*this)(f, std::move(x), y.begin(), y.end());
        }

        template <class Y>
        O operator()(bfun_t f, L<Y> y) const {
            if (y.empty()) throw Exception("length: unary over on empty list");
            Object b(box(y[0]));
            return (*this)(f, O(&b), y.begin() + 1, y.end());
        }

        O operator()(bfun_t f, L<O> y) const {
            if (y.empty()) throw Exception("length: unary over on empty list");
            return (*this)(f, y[0], y.begin() + 1, y.end());
        }

        O operator()(bfun_t f, O y) const {
#define CS(X) case OT<X>::typei(): return (*this)(f, L<X>(std::move(y)))
            switch (int(y->type)) {
            CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X); CS(O);
            case '!': return (*this)(f, UKV(std::move(y)).val());
            default: throw Exception("type: over's rhs must be list or dict");
            }
#undef CS
        }

        O operator()(bfun_t f, O x, O y) const {
#define CS(X) case OT<X>::typei(): return (*this)(f, std::move(x), L<X>(std::move(y)))
            switch (int(y->type)) {
            CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X); CS(O);
            case '!': return (*this)(f, std::move(x), UKV(std::move(y)).val());
            default: throw Exception("type: over's rhs must be list or dict");
            }
#undef CS
        }
    } over_;
}

O over(bfun_t f, O x)      { return over_(f, x); }
O over(bfun_t f, O x, O y) { return over_(f, x, y); }
