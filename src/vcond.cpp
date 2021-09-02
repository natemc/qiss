#include <vcond.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <ul.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct VCond {
        template <class X> L<X> operator()(L<B> x, L<X> y, L<X> z) const {
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
            L<X> r;
            r.reserve(x.size());
            for (index_t i = 0; i < x.size(); ++i) r.emplace_back((x[i]? y:z)[i]);
            return r;
        }
    } vcond_;
}

O vcond(O x, O y, O z) {
    if (x.type() != OT<B>::typet() || y.type() != z.type() || !y.is_list())
        throw Exception("type: ? (vector conditional)");
    if (UL(x).size() != UL(y).size() || UL(y).size() != UL(z).size())
        throw Exception("length: ? (vector conditional)");

    L<B> b(std::move(x));
#define CS(X) case OT<X>::typei(): return vcond_(           \
        std::move(b), L<X>(std::move(y)), L<X>(std::move(z)))
    switch (int(y.type())) {
    CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X); CS(O);
    default:  throw Exception("type: ? (vector conditional)");
    }
#undef CS
}
