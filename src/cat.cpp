#include <cat.h>
#include <algorithm>
#include <exception.h>
#include <flip.h>
#include <iterator>
#include <kv.h>
#include <l.h>
#include <lcutil.h>
#include <o.h>
#include <objectio.h>
#include <type_pair.h>
#include <ukv.h>

#define DXX(X,Y)                                                                \
case TypePair<X,Y>::AA: return (*this)(x.atom<X>()       , y.atom<Y>());        \
case TypePair<X,Y>::AL: return (*this)(x.atom<X>()       , L<Y>(std::move(y))); \
case TypePair<X,Y>::LA: return (*this)(L<X>(std::move(x)), y.atom<Y>());        \
case TypePair<X,Y>::LL: return (*this)(L<X>(std::move(x)), L<Y>(std::move(y)))
#define DXY(X,Y) DXX(X,Y); DXX(Y,X)
#define DXO(X)                                                                  \
case TypePair<X,O>::AL: return (*this)(x.atom<X>()       , L<O>(std::move(y))); \
case TypePair<X,O>::LL: return (*this)(L<X>(std::move(x)), L<O>(std::move(y))); \
case TypePair<O,X>::LA: return (*this)(L<O>(std::move(x)), y.atom<X>());        \
case TypePair<O,X>::LL: return (*this)(L<O>(std::move(x)), L<X>(std::move(y)))

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Cat {
        template <class X> auto operator()(X x, X y) const { return L<X>{x, y}; }
    
        template <class X> auto operator()(X x, L<X> y) const {
            if (y.mine()) {
                y.emplace_back(x);
                std::rotate(y.begin(), y.end() - 1, y.end());
                return y;
            }
            L<X> r(y.size() + 1);
            r[0] = x;
            std::copy(y.begin(), y.end(), r.begin() + 1);
            return r;
        }
    
        template <class X> auto operator()(L<X> x, X y) const {
            if (x.mine()) {
                x.emplace_back(y);
                return x;
            }
            L<X> r(x.size() + 1);
            std::copy(x.begin(), x.end(), r.begin());
            r.back() = y;
            return r;
        }
    
        template <class X> auto operator()(L<X> x, L<X> y) const {
            if (x.mine()) {
                x.append(y.begin(), y.end());
                return x;
            }
            L<X> r(x.size() + y.size());
            std::copy(x.begin(), x.end(), r.begin());
            std::copy(y.begin(), y.end(), r.begin() + x.size());
            return r;
        }
    
        template <class X, class Y> auto operator()(X x, Y y) const {
            return L<O>{O(x), O(y)};
        }
    
        template <class X, class Y> O operator()(X x, L<Y> y) const {
            if (y.empty()) return L<X>{x};
            L<O> r;
            r.reserve(y.size() + 1);
            r.emplace_back(x);
            r.append(y.begin(), y.end());
            return std::move(r);
        }
    
        template <class X, class Y> O operator()(L<X> x, Y y) const {
            if (x.empty()) return L<Y>{y};
            L<O> r;
            r.reserve(x.size() + 1);
            r.append(x.begin(), x.end());
            r.emplace_back(y);
            return std::move(r);
        }
    
        template <class X, class Y> auto operator()(L<X> x, L<Y> y) const {
            L<O> r;
            r.reserve(x.size() + y.size());
            r.append(x.begin(), x.end());
            r.append(y.begin(), y.end());
            return r;
        }
    
        UKV operator()(UKV, UKV) const {
            throw Exception("nyi , on dicts");
        }
    
        O tables(O x, O y) const {
            KV<S,O> top(addref(x->dict));
            KV<S,O> bot(addref(y->dict));
            if (top.size() != bot.size())
                throw Exception("mismatch: , on tables with different columns");
            for (index_t i = 0; i < top.size(); ++i) {
                O bv(bot.at(top.key()[i], O()));
                if (bv.type() == generic_null_type || bv.type() != top.val()[i]->type)
                    throw Exception("mismatch: , on tables with different columns");
            }
            L<S> k(top.key());
            L<O> v;
            v.reserve(top.size());
            for (index_t i = 0; i < k.size(); ++i)
                v.emplace_back((*this)(top.val()[i], bot[k[i]]));
            return +UKV(std::move(k), std::move(v));
        }
    
        O operator()(O x, O y) const {
            switch (type_pair(x, y)) {        
            DXX(B,B); DXY(B,C); DXY(B,D); DXY(B,F); DXY(B,H); DXY(B,I); DXY(B,J);
            DXY(B,S); DXY(B,T); DXY(B,X); DXO(B);
            DXX(C,C); DXY(C,D); DXY(C,F); DXY(C,H); DXY(C,I); DXY(C,J); DXY(C,S);
            DXY(C,T); DXY(C,X); DXO(C);
            DXX(D,D); DXY(D,F); DXY(D,H); DXY(D,I); DXY(D,J); DXY(D,S); DXY(D,T);
            DXY(D,X); DXO(D);
            DXX(F,F); DXY(F,H); DXY(F,I); DXY(F,J); DXY(F,S); DXY(F,T); DXY(F,X);
            DXO(F);
            DXX(H,H); DXY(H,I); DXY(H,J); DXY(H,S); DXY(H,T); DXY(H,X); DXO(H);
            DXX(I,I); DXY(I,J); DXY(I,S); DXY(I,T); DXY(I,X); DXO(I);
            DXX(J,J); DXY(J,S); DXY(J,T); DXY(J,X); DXO(J);
            DXX(S,S); DXY(S,T); DXY(S,X); DXO(S);
            DXX(T,T); DXY(T,X); DXO(T);
            DXX(X,X); DXO(X);
            case TypePair<O,O>::LL:
                return (*this)(L<O>(std::move(x)), L<O>(std::move(y)));
            case ('!' << 8) | '!': return (*this)(UKV(std::move(x)), UKV(std::move(y)));
            case ('+' << 8) | '+': return tables(x, y);
            default: {
                L<C> s;
                s << "nyi: , for types " << x.type() << " and " << y.type();
                throw lc2ex(s);
            }
            }
        }
    } cat_;
} // unnamed

O cat(O x, O y) { return cat_(std::move(x), std::move(y)); }
