#include <scan.h>
#include <enlist.h>
#include <exception.h>
#include <l.h>
#include <lambda.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Scan {
        template <class FUN, class R, class Y>
        O operator()(FUN f, R r0, R r1, Y* first, Y* last) const {
            const index_t n = std::distance(first, last);
            L<R> r(n);
            r[0] = r0;
            r[1] = r1;
            for (index_t i = 2; i < n; ++i) {
                O rx(f(O(r[i-1]), first[i]));
                if (rx.type() == -OT<R>::typet())
                    r[i] = rx.atom<R>();
                else {
                    L<O> p;
                    p.reserve(n);
                    for (index_t j = 0; j < i; ++j) p.emplace_back(r[j]);
                    p.emplace_back(std::move(rx));
                    for (; i < n; ++i) p.emplace_back(f(p[i-1], first[i]));
                    return O(std::move(p));
                }
            }
            return O(std::move(r));
        }

        template <class FUN, class Y>
        O operator()(FUN f, O x, Y* first, Y* last) const {
            const index_t n = std::distance(first, last);
            if (!n) return enlist(x);
            O r0(f(std::move(x), *first));
            if (first + 1 == last) return enlist(std::move(r0));
            O r1(f(r0, first[1]));
            if (!r0.is_atom() || r0.type() != r1.type()) {
                L<O> r;
                r.reserve(n);
                r.emplace_back(std::move(r0));
                r.emplace_back(std::move(r1));
                for (index_t i = 2; i < n; ++i)
                    r.emplace_back(f(r[i-1], first[i]));
                return O(std::move(r));
            }
#define CS(X) case -OT<X>::typei(): \
    return (*this)(f, r0.atom<X>(), r1.atom<X>(), first, last)
            switch (int(r0.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X);
            default: assert(false);
            }
#undef CS
            return O{}; // not reached
        }

        template <class Y>
        O operator()(bfun_t f, L<Y> y) const {
            if (y.empty()) throw Exception("length: unary scan on empty list");
            return (*this)(L2(f(x, O(y))), O(y[0]), y.begin() + 1, y.end());
        }

        O operator()(bfun_t f, L<O> y) const {
            if (y.empty()) throw Exception("length: unary scan on empty list");
            return (*this)(f, y[0], y.begin() + 1, y.end());
        }

        O operator()(bfun_t f, O y) const {
#define CS(X) case OT<X>::typei(): return (*this)(f, L<X>(std::move(y)))
            switch (int(y.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X); CS(O);
            case '!': {
                UKV d(std::move(y));
                return UKV(d.key(), (*this)(f, d.val()));
            }
            default: throw Exception("type: over's rhs must be list or dict");
            }
#undef CS
        }

        template <class Y>
        O operator()(bfun_t f, O x, L<Y> y) const {
            return (*this)(L2(f(x, O(y))), std::move(x), y.begin(), y.end());
        }

        O operator()(bfun_t f, O x, L<O> y) const {
            return (*this)(f, std::move(x), y.begin(), y.end());
        }

        O operator()(bfun_t f, O x, O y) const {
#define CS(X) case OT<X>::typei(): return (*this)(f, std::move(x), L<X>(std::move(y)))
            switch (int(y.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X); CS(O);
            case '!': {
                UKV d(std::move(y));
                return UKV(d.key(), (*this)(f, std::move(x), d.val()));
            }
            default: throw Exception("type: scan's rhs must be list or dict");
            }
#undef CS
        }
    } scan_;
} // unnamed

O scan(bfun_t f, O x)      { return scan_(f, x); }
O scan(bfun_t f, O x, O y) { return scan_(f, x, y); }
