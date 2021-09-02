#include <key.h>
#include <exception.h>
#include <limits>
#include <numeric>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class X> L<X> iota(index_t n) {
        L<X> r(n);
        std::iota(r.begin(), r.end(), X(0));
        return r;
    }
}

L<J> Til::operator()(index_t n) const { return iota<J>(n); }

L<J> Til::operator()(O x) const {
#define ATOM(X) case -OT<X>::typei(): return (*this)(X::rep(x.atom<X>()))
    switch (int(x.type())) {
    ATOM(B); ATOM(I); ATOM(J); ATOM(X);
    default:
        if (x.is_list()) return (*this)(x.get()->n);
        throw Exception("nyi: til");
    }
#define ATOM(X) case -OT<X>::typei(): return (*this)(X::rep(x.atom<X>()))
}

L<I> TilI::operator()(index_t n) const {
    if (std::numeric_limits<I::rep>::max() < n) throw Exception("length (tili)");
    return iota<I>(n);
}

L<I> TilI::operator()(O x) const {
#define ATOM(X) case -OT<X>::typei(): return (*this)(X::rep(x.atom<X>()))
    switch (int(x.type())) {
    ATOM(B); ATOM(I); ATOM(J); ATOM(X);
    default:
        if (x.is_list()) return (*this)(x.get()->n);
        throw Exception("nyi: tili");
    }
#undef ATOM
}

O key(O x) {
    if (x.is_atom()) return til(std::move(x));
    if (x.is_dict()) return UKV(std::move(x)).key();
//    if (x.is_list()) return til(UL(std::move(x)).size());
    if (x.is_list()) return til(x.get()->n);
    throw Exception("nyi: key");
}
