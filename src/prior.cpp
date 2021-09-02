#include <prior.h>
#include <enlist.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Prior {
        template <class FUN, class R, class Y>
        O operator()(FUN f, R r0, R r1, Y* first, Y* last) const {
            const index_t n = std::distance(first, last);
            L<R> r(n);
            r[0] = r0;
            r[1] = r1;
            for (index_t i = 2; i < n; ++i) {
                O rx(f(first[i], first[i-1]));
                if (rx.type() == -OT<R>::typet())
                    r[i] = rx.atom<R>();
                else {
                    L<O> p;
                    p.reserve(n);
                    for (index_t j = 0; j < i; ++j) p.emplace_back(r[j]);
                    p.emplace_back(std::move(rx));
                    for (; i < n; ++i) p.emplace_back(f(first[i], first[i-1]));
                    return O(std::move(p));
                }
            }
            return O(std::move(r));
        }

        template <class FUN, class Y>
        O operator()(FUN f, O r0, Y* first, Y* last) const {
            const index_t n = std::distance(first, last);
            O r1(f(first[1], first[0]));
            if (!r0.is_atom() || r0.type() != r1.type()) {
                L<O> r;
                r.reserve(n);
                r.emplace_back(std::move(r0));
                r.emplace_back(std::move(r1));
                for (index_t i = 2; i < n; ++i)
                    r.emplace_back(f(first[i], first[i-1]));
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
        O operator()(bfun_t f, O x, Y* first, Y* last) const {
            if (first == last) return enlist(x);
            Object boxb(box(*first));
            O r0(f(O(&boxb), std::move(x)));
            if (first + 1 == last) return enlist(std::move(r0));
            auto g = [&](Y a, Y b){
                Object ba(box(a));
                Object bb(box(b));
                return f(O(&ba), O(&bb));
            };
            return (*this)(g, std::move(r0), first, last);
        }

        O operator()(bfun_t f, O x, O* first, O* last) const {
            if (first == last) return enlist(x);
            O r0(f(*first, x));
            if (first + 1 == last) return enlist(std::move(r0));
            return (*this)(f, std::move(r0), first, last);
        }

        template <class Y>
        O operator()(bfun_t f, L<Y> y) const {
            if (y.empty()) throw Exception("length: unary scan on empty list");
            // We cannot call the overload on line 56 here, because
            // then the result would be one element short.
            Object box0(box(y[0]));
            auto g = [&](Y a, Y b){
                Object ba(box(a));
                Object bb(box(b));
                return f(O(&ba), O(&bb));
            };
            return (*this)(g, O(&box0), y.begin() + 1, y.end());
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
            return (*this)(f, std::move(x), y.begin(), y.end());
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
    } prior_;
}

O prior(bfun_t f, O x)      { return prior_(f, x); }
O prior(bfun_t f, O x, O y) { return prior_(f, x, y); }
