#include <not.h>
#include <each.h>
#include <exception.h>
#include <flip.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Atom {
        template <class X> B operator()(X x) const { return B(!bool(x)); }
    } atom;
}

O not_(O x) {
#define CS(X) case -OT<X>::typei(): return O(atom(x.atom<X>())); \
              case OT<X>::typei() : return each(atom, L<X>(std::move(x)))
    switch (int(x.type())) {
    CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
    case OT<O>::typei(): return each(not_, L<O>(std::move(x)));
    case '!': {
        UKV d(std::move(x));
        return UKV(d.key(), not_(d.val()));
    }
    case '+': return +not_(+std::move(x));
    default: throw Exception("type (~ not)");
    }
#undef CS
}
