#include <find.h>
#include <at.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <each.h>
#include <exception.h>
#include <indexof.h>
#include <key.h>
#include <match.h>
#include <l.h>
#include <o.h>
#include <objectio.h>
#include <prim_parse.h>
#include <sym.h>
#include <type_pair.h>
#include <ukv.h>
#include <utility>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    int _rand() { return rand(); } // NOLINT

    L<J> deal(index_t n, index_t bound) {
        assert(0 <= n && 0 <= bound);
        if (bound < n) throw Exception("length: ? (deal)");
        L<J> p(til(bound));
        // TODO if n is close to bound, use std::shuffle
        L<J>  r(n);
        for (index_t i(0); i<n; ++i) {
            const index_t j(_rand() % bound--);
            r[i] = p[j];
            std::swap(p[j], p[bound]);
        }
        return r;
    }

    L<J> rand_(index_t n, index_t bound) {
        assert(0 <= n && 0 <= bound);
        L<J> r(n);
        for (index_t i(0); i<n; ++i)
            r[i] = J(J::rep((double(_rand()) / RAND_MAX) * double(bound)));
        return r;
    }

    L<S> rand_(index_t n, S len) {
        const char* const s = c_str(len);
        const C* const c = reinterpret_cast<const C*>(s);
        const auto [ok, k] = parse_long(c, c + strlen(s));
        if (!ok) throw Exception("type: rand rhs");
        char buf[256];
        if (k == J(0) || J(sizeof buf) <= k) throw Exception("domain: rand rhs");
        buf[J::rep(k)] = '\0';
        L<S> r(n);
        for (index_t i = 0; i < n; ++i) {
            for (index_t j = 0; J(j) < k; ++j)
                buf[j] = char('a' + int(26 * (double(_rand()) / RAND_MAX)));
            r[i] = sym(buf);
        }
        return r;
    }

    const struct Find {
        // rand
        template <class Y>
        L<Y> operator()(J x, L<Y> y) const {
            L<J> i(J::rep(x) < 0? deal(-J::rep(x), y.size()) : rand_(J::rep(x), y.size()));
            return L<Y>(at(std::move(y), i));
        }

        L<J> operator()(J x, J y) const {
            if (y < J(1)) throw Exception("? rhs must be positive");
            return J::rep(x) < 0? deal(-J::rep(x), J::rep(y))
                 :                rand_(J::rep(x), J::rep(y));
        }

        L<S> operator()(J x, S y) const {
            if (x < J(0)) throw Exception("nyi: ? (deal) for sym rhs");
            return rand_(J::rep(x), y);
        }

        // find
        template <class X>
        J operator()(L<X> x, X y) const { return J(indexof(x, y)); }

        template <class X> L<J> operator()(L<X> x, L<X> y) const {
            return L<J>(eachR(*this, std::move(x), std::move(y)).release());
        }

        template <class X> L<O> operator()(L<X> x, L<O> y) const {
            return L<O>(eachR(*this, std::move(x), std::move(y)).release());
        }

        L<J> operator()(L<O> x, L<O> y) const {
            return L<J>(eachR(*this, std::move(x), std::move(y)).release());
        }

        O operator()(O x, O y) const {
            using std::move;
            if (x.is_dict()) {
                UKV d(std::move(x));
                return at(d.key(), (*this)(d.val(), move(y)));
            }
            if (x.is_table()) {
            }
#define FIND(X)                                                         \
case TypePair<X,X>::LA: return O((*this)(L<X>(move(x)), y.atom<X>()));  \
case TypePair<X,X>::LL: return   (*this)(L<X>(move(x)), L<X>(move(y))); \
case TypePair<X,O>::LL: return   (*this)(L<X>(move(x)), L<O>(move(y))); \
case TypePair<O,X>::LA: return O((*this)(L<O>(move(x)), y));            \
case TypePair<O,X>::LL: return O((*this)(L<O>(move(x)), y))
#define RAND(X) case TypePair<J,X>::AL: return (*this)(x.atom<J>(), L<X>(move(y)))
            switch (type_pair(x, y)) {
            case TypePair<J,J>::AA: return (*this)(x.atom<J>(), y.atom<J>());
            FIND(B); FIND(C); FIND(D); FIND(F); FIND(H);
            FIND(J); FIND(S); FIND(T); FIND(X);
            RAND(B); RAND(C); RAND(D); RAND(F); RAND(H);
            RAND(J); RAND(S); RAND(T); RAND(X);
            case TypePair<J,S>::AA: return (*this)(x.atom<J>(), y.atom<S>());
            case TypePair<O,O>::LL: return (*this)(L<O>(move(x)), L<O>(move(y)));
            default: {
                L<C> s;
                s << "nyi ? for types " << x.type() << " and " << y.type();
                throw lc2ex(s);
            }
            }
#undef RAND
#undef FIND
        }
    } find_;
} // unnamed

O find(O x, O y) { return find_(std::move(x), std::move(y)); }
