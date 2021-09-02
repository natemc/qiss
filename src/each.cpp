#include <each.h>
#include <enlist.h>
#include <exception.h>
#include <o.h>
#include <type_pair.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct EachBox1 {
        template <class FUN, class X> O boxing(FUN f, L<X> x) const {
            return each([&](X e){ Object b(box(e)); return f(O(&b)); }, std::move(x));
        }

        template <class F> O operator()(F&& f, Dict* x) const {
            return O(make_dict(addref(dk(x)),
                               (*this)(std::forward<F>(f), O(addref(dv(x)))).release()));
        }

        template <class FUN> O operator()(FUN&& f, O x) const {
            if (x.is_dict()) return (*this)(std::forward<FUN>(f), dict(x.get()));
#define CS(X) case OT<X>::typei(): \
    return boxing(std::forward<FUN>(f), L<X>(std::move(x)))
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
            CS(S); CS(T); CS(X);
            case OT<O>::typei():
                return each(std::forward<FUN>(f), L<O>(std::move(x)));
            // TODO handle dict, table
            default: throw Exception("nyi unary each");
            }
#undef CS
        }
    } eachbox1;

    const struct EachBox2 {
        template <class X, class Y>
        O boxboth(bfun_t f, L<X> x, L<Y> y) const {
            auto g = [&](X a, Y b){
                Object boxx(box(a));
                Object boxy(box(b));
                return f(O(&boxx), O(&boxy));
            };
            return each(g, x, y);
        }

        template <class X>
        O boxx(bfun_t f, L<X> x, L<O> y) const {
            auto g = [&](X a, O b){
                Object boxx(box(a));
                return f(O(&boxx), std::move(b));
            };
            return each(g, std::move(x), std::move(y));
        }

        template <class Y>
        O boxy(bfun_t f, L<O> x, L<Y> y) const {
            auto g = [&](O a, Y b){
                Object boxy(box(b));
                return f(std::move(a), O(&boxy));
            };
            return each(g, std::move(x), std::move(y));
        }

        O operator()(bfun_t f, O x, O y) const {
            if      (x.is_atom())
                return y.is_atom()? f(std::move(x), std::move(y))
                    :  /* else */   each_right(f, std::move(x), std::move(y));
            else if (y.is_atom()) return each_left(f, std::move(x), std::move(y));
#define BB(X,Y)                                                                        \
    case TypePair<X,Y>::LL: return boxboth(f, L<X>(std::move(x)), L<Y>(std::move(y)))
#define BO(X)                                                                          \
    case TypePair<X,O>::LL: return boxx   (f, L<X>(std::move(x)), L<O>(std::move(y))); \
    case TypePair<O,X>::LL: return boxy   (f, L<O>(std::move(x)), L<X>(std::move(y)))
#define AA(Z) BB(Z,B); BB(Z,C); BB(Z,D); BB(Z,F); BB(Z,H); BB(Z,I); \
              BB(Z,J); BB(Z,S); BB(Z,T); BB(Z,X)
            switch (type_pair(x, y)) {
            AA(B); AA(C); AA(D); AA(F); AA(H); AA(I); AA(J);
            AA(S); AA(T); AA(X);
            BO(B); BO(C); BO(D); BO(F); BO(H); BO(I); BO(J);
            BO(S); BO(T); BO(X);
            case TypePair<O,O>::LL:
                return each(f, L<O>(std::move(x)), L<O>(std::move(y)));
            // TODO handle dict, table
            default: throw Exception("nyi binary each");
            }
#undef AA
#undef BB
        }
    } eachbox2;
} // unnamed

O each1(ufun_t f, O x) { return eachbox1(f, std::move(x)); }

O each2(bfun_t f, O x, O y) { return eachbox2(f, std::move(x), std::move(y)); }

O each_left(bfun_t f, O x, O y) {
    return eachbox1([&](O a){ return f(std::move(a), y); }, std::move(x));
}

O each_right(bfun_t f, O x, O y) {
    return eachbox1([&](O b){ return f(x, std::move(b)); }, std::move(y));
}
