#include <vcond.h>
#include <cmath>
#include <exception.h>
#include <format.h>
#include <l.h>
#include <lcutil.h>
#include <o.h>
#include <objectio.h>
#include <type_pair.h>
#include <ul.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;
    template <class X, class Y> using TP = TypePair<X, Y>;

    const struct VCond {
        template <class Z> L<Z> operator()(L<B> x, Z y, Z z) const {
            L<Z> r;
            r.reserve(x.size());
            for (index_t i = 0; i < x.size(); ++i) r.emplace_back(x[i]? y:z);
            return r;
        }

        template <class Z> L<Z> operator()(L<B> x, Z y, L<Z> z) const {
            if (z.mine()) {
                for (index_t i = 0; i < x.size(); ++i)
                    if (x[i]) z[i] = y;
                return z;
            }
            L<Z> r;
            r.reserve(x.size());
            for (index_t i = 0; i < x.size(); ++i) r.emplace_back(x[i]? y:z[i]);
            return r;
        }

        template <class Z> L<Z> operator()(L<B> x, L<Z> y, Z z) const {
            if (y.mine()) {
                for (index_t i = 0; i < x.size(); ++i)
                    if (!x[i]) y[i] = z;
                return y;
            }
            L<Z> r;
            r.reserve(x.size());
            for (index_t i = 0; i < x.size(); ++i) r.emplace_back(x[i]? y[i]:z);
            return r;
        }

        template <class Z> L<Z> operator()(L<B> x, L<Z> y, L<Z> z) const {
            // TODO write ternary each
            if (y.mine()) {
                for (index_t i = 0; i < x.size(); ++i)
                    if (!x[i]) y[i] = z[i];
                return y;
            }
            if (z.mine()) {
                for (index_t i = 0; i < x.size(); ++i)
                    if (x[i]) z[i] = y[i];
                return z;
            }
            L<Z> r;
            r.reserve(x.size());
            for (index_t i = 0; i < x.size(); ++i) r.emplace_back((x[i]? y:z)[i]);
            return r;
        }
    } vcond_;

    Exception type_error(O x, O y, O z) {
        L<C> s;
        s << "type: ? (vector conditional): " << x.type() << y.type() << z.type();
        throw lc2ex(s);
    }
}

O vcond(O x, O y, O z) {
    if (x.type() != OT<B>::typet() || std::abs(int(y.type())) != std::abs(int(z.type())))
        throw type_error(x, y, z);
    if (y.is_list() && y->n != x->n || z.is_list() && z->n != x->n)
        throw Exception("length: ? (vector conditional)");

    L<B> b(std::move(x));
#define CS(Z)                                                                          \
case TP<Z,Z>::AA: return vcond_(std::move(b), y.atom<Z>()       , z.atom<Z>());        \
case TP<Z,Z>::AL: return vcond_(std::move(b), y.atom<Z>()       , L<Z>(std::move(z))); \
case TP<Z,Z>::LA: return vcond_(std::move(b), L<Z>(std::move(y)), z.atom<Z>());        \
case TP<Z,Z>::LL: return vcond_(std::move(b), L<Z>(std::move(y)), L<Z>(std::move(z)));
    switch (type_pair(y, z)) {
    CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
    case TP<O,O>::LL: return vcond_(std::move(b), L<O>(std::move(y)), L<O>(std::move(z)));
    default         :  throw type_error(x, y, z);
    }
#undef CS
}
