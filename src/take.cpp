#include <take.h>
#include <algorithm>
#include <at.h>
#include <each.h>
#include <exception.h>
#include <flip.h>
#include <o.h>
#include <objectio.h>
#include <primio.h>
#include <sym.h>
#include <type_pair.h>
#include <utility>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    L<O> replicate(index_t n, O x) {
        L<O> r;
        r.reserve(n);
        std::fill_n(std::back_inserter(r), n, x);
        return r;
    }

    // Returns rows, cols, and cols in last row
    template <class X>
    std::tuple<index_t, index_t, index_t> rows_and_cols(L<X> shape, index_t n) {
        const index_t rows = typename X::rep(shape[0]);
        const index_t cols = typename X::rep(shape[1]);
        if (shape[0].is_null()) {
            if (shape[1].is_null())
                throw Exception("pair of nulls on lhs of #");
            const index_t r             = n / cols + (0 < n % cols);
            const index_t last_row_cols = n  - (r - 1) * cols;
            return {r, cols, last_row_cols};
        } else if (shape[1].is_null()) {
            const index_t c             = n / rows;
            const index_t last_row_cols = n - (rows - 1) * c;
            return {rows, c, last_row_cols};
        }
        return {rows, cols, cols};
    }

    template <class X, class Y> O reshape(L<X> shape, L<Y> y) {
        if (shape.size() != 2) {
            // TODO k treats a list of length 1 as if it were an atom, e.g.,
            // 0 1~(,2)#!4     Should qiss do the same?
            L<C> s;
            s << "length: reshape lhs expected 2 values, saw " << shape;
            throw lc2ex(s);
        }

        const auto [rows, cols, last_row_cols] = rows_and_cols(shape, y.size());
        index_t i(0);
        auto make_row = [&](index_t width){
            L<Y> row;
            row.reserve(width);
            for (index_t c(0); c<width; ++c) row.emplace_back(y[i++ % y.size()]);
            return row;
        };

        L<O> r;
        r.reserve(rows);
        // clang doesn't like capture of cols, so we have to write cols=cols
        // See https://stackoverflow.com/questions/46114214
        std::generate_n(std::back_inserter(r), rows - 1,
                        [&,cols=cols](){ return make_row(cols); });
        r.emplace_back(make_row(last_row_cols));
        return O(std::move(r));
    }

    template <class X, class Y>
    O reprect(L<X> shape, Y y) {
        if (shape.size() != 2) throw Exception("length: #");
        const typename X::rep rows(shape[0]);
        const typename X::rep cols(shape[1]);
        L<O> r;
        r.reserve(rows);
        std::generate_n(std::back_inserter(r), rows, [&](){ return replicate(cols, y); });
        return O(std::move(r));
    }
  
    template <class Y> L<Y> take_from_back(index_t n, L<Y> y) {
        L<Y> r;
        r.reserve(n);
        if (n <= y.size())
            for (index_t i(0); i<n; ++i) r.emplace_back(y[y.size() - n + i]);
        else {
            const index_t start(y.size() - n % y.size());
            for (index_t i(0); i<n; ++i) r.emplace_back(y[(start+i) % y.size()]);
        }
        return r;
    }

    template <class Y>
    L<Y> take_(index_t n, L<Y> y) {
        if (J(n).is_null()) throw Exception("can't take 0N elements");
        if (n < 0) return take_from_back(-n, y);
        L<Y> r;
        r.reserve(n);
        if (0 == y.size())
            for (index_t i(0); i<n; ++i) r.emplace_back(OT<Y>::null());
        else
            for (index_t i(0); i<n; ++i) r.emplace_back(y[i % y.size()]);
        return r;
    }

    L<O> take_each(O n, L<O> y) {
        return L<O>(eachR(take, std::move(n), std::move(y)));
    }

    template <class Y>
    L<Y> apply_attr(S x, L<Y> y) {
        switch (c_str(x)[0]) {
        case 's':
            for (index_t i = 1; i < y.size(); ++i)
                if (y[i] < y[i - 1])
                    throw Exception("domain: can't apply s attr to unsorted list");
            y.get()->a = Attr::sorted;
            break;
        default : throw Exception("nyi # (apply attr)");
        }
        return y;
    }
} // unnamed

O take(O x, O y) {
    if (y.is_dict()) {
        const Type xt(int8_t(std::abs(int(x.type()))));
        if (xt == OT<B>::typet() || xt == OT<I>::typet() || xt == OT<J>::typet()) {
            auto [yk, yv] = UKV(std::move(y)).kv();
            auto rhs      = take(x, std::move(yv));
            return UKV(take(std::move(x), std::move(yk)), std::move(rhs));
        }
        if (x.type() == -OT<S>::typet())
            return take(L<S>{x.atom<S>()}, std::move(y));
        if (x.type() == OT<S>::typet()) {
            auto rhs = at(std::move(y), x);
            return UKV(std::move(x), std::move(rhs));
        }
        throw Exception("type (# from dict)");
    }

    if (y.is_table()) {
        UKV d(+std::move(y));
        if (x.type() == -OT<B>::typet() || x.type() == -OT<I>::typet() ||
            x.type() == -OT<J>::typet())
        {
            auto [k, v] = std::move(d).kv();
            return +UKV(std::move(k), take_each(std::move(x), L<O>(std::move(v))));
        }
        if (x.type() == OT<B>::typet() || x.type() == OT<I>::typet() ||
            x.type() == OT<J>::typet())
        {
            if (x->n != 2) throw Exception("length (# from table)");
            // TODO 2 2#([]a:1 2 3 4) => (([]a:1 2); ([]a:3 4))
            throw Exception("nyi (# reshape table)");
        }
        if (x.type() == OT<S>::typet() || x.type() == -OT<S>::typet())
            return +UKV(take(std::move(x), std::move(d)));
        throw Exception("type (# from table)");
    }

#define CO(X)                                                                    \
case TypePair<X,O>::AL: return take_  (X::rep(x.atom<X>()), L<O>(std::move(y))); \
case TypePair<X,O>::LL: return reshape(L<X>(std::move(x)) , L<O>(std::move(y)))
#define CS(X,Y)                                                                    \
case TypePair<X,Y>::AA: return replicate(X::rep(x.atom<X>()), y.atom<X>());        \
case TypePair<X,Y>::AL: return take_    (X::rep(x.atom<X>()), L<Y>(std::move(y))); \
case TypePair<X,Y>::LA: return reprect  (L<X>(std::move(x)) , y.atom<Y>());        \
case TypePair<X,Y>::LL: return reshape  (L<X>(std::move(x)) , L<Y>(std::move(y)))
#define AT(X) case TypePair<S,X>::AL: return apply_attr(x.atom<S>(), L<X>(std::move(y)))
#define NUL(X) case type_pair(-OT<X>::typet(), generic_null_type): \
        return replicate(X::rep(x.atom<X>()), std::move(y))
    switch (type_pair(x, y)) {
    CS(B,B); CS(B,C); CS(B,D); CS(B,F); CS(B,H); CS(B,I); CS(B,J);
    CS(B,S); CS(B,T); CS(B,X); CO(B);
    CS(I,B); CS(I,C); CS(I,D); CS(I,F); CS(I,H); CS(I,I); CS(I,J);
    CS(I,S); CS(I,T); CS(I,X); CO(I);
    CS(J,B); CS(J,C); CS(J,D); CS(J,F); CS(J,H); CS(J,I); CS(J,J);
    CS(J,S); CS(J,T); CS(J,X); CO(J);
    AT(B); AT(C); AT(D); AT(F); AT(H); AT(I); AT(J); AT(S); AT(T); AT(X);
    NUL(B); NUL(I); NUL(J);
    default: {
        L<C> s;
        s << "nyi # for types " << x.type() << " and " << y.type();
        throw lc2ex(s);
    }
    }
#undef NUL
#undef AT
#undef CS
#undef CO
}
