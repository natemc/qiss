#include <in.h>
#include <algorithm>
#include <each.h>
#include <hashset.h>
#include <indexof.h>
#include <match.h>
#include <l.h>
#include <o.h>
#include <type_traits>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class Y> struct In {
        explicit In(L<Y> y): h(y) {
            for (index_t i = 0; i < y.size(); ++i) h.insert(i);
        }
        template <class X> O atom(O x) const {
            if constexpr(std::is_same_v<X,Y>) return O(B(h.has(x.atom<X>())));
            else                              throw Exception("type (in)");
        }
        template <class X> O list(O x) const {
            if constexpr(std::is_same_v<X,Y>)
                return each([&](Y e){ return B(h.has(e)); }, L<Y>(std::move(x)));
            else 
                throw Exception("type (in)");
        }
        O operator()(O x) const {
            switch (int(x.type())) {
            case -OT<Y>::typei(): return atom<Y>(std::move(x));
            case OT<Y>::typei() : return list<Y>(std::move(x));
            case OT<O>::typei() : return each(*this, L<O>(x));
            default             : throw Exception("type (in)");
            }
        }
    private:
        HashSet<Y> h;
    };

    template <> struct In<O> {
        explicit In(L<O> y): h(y) {
            for (index_t i = 0; i < y.size(); ++i) h.insert(i);
        }

        template <class X> O list(L<X> x) const {
            auto f = [&](X e) {
                Object b(box(e));
                return B(h.has(O(&b)));
            };
            return each(f, std::move(x));
        }

        O operator()(O x) const {
#define CS(X) case OT<X>::typei() : return list(L<X>(std::move(x))); \
              case -OT<X>::typei(): return O(B(h.has(std::move(x))))
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
            case OT<O>::typei() : return each(*this, L<O>(std::move(x)));
            default             : throw Exception("type (in)");
            }
#undef CS
        }
    private:
        HashSet<O> h;
    };

    template <class Y> O in_(O x, L<Y> y) {
        return In<Y>(std::move(y))(std::move(x));
    }
}

O in(O x, O y) {
#define ATOM(Y) case -OT<Y>::typei():                                \
    if (x.type() != y.type()) throw Exception("type (in");           \
    else                      return O(B(x.atom<Y>() == y.atom<Y>()));
#define LIST(Y) case OT<Y>::typei() : return in_(std::move(x), L<Y>(std::move(y)));
    switch (int(y.type())) {
    ATOM(B); ATOM(C); ATOM(D); ATOM(F); ATOM(H); ATOM(I); ATOM(J); ATOM(S); ATOM(T); ATOM(X);
    LIST(B); LIST(C); LIST(D); LIST(F); LIST(H); LIST(I); LIST(J); LIST(S); LIST(T); LIST(X);
    LIST(O);
    case '!': return in(std::move(x), UKV(std::move(y)).val());
    default : throw Exception("nyi: in");
    }
}
