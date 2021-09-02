#include <group.h>
#include <at.h>
#include <enlist.h>
#include <exception.h>
#include <flip.h>
#include <grouper.h>
#include <l.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class X> UKV group_(L<X> x) {
        Grouper<X> g(std::move(x));
        return UKV(g.key(), g.val());
    }
} // unnamed

O group(O x) {
#define CS(X) case OT<X>::typei(): return group_(L<X>(std::move(x)))
    switch (int(x->type)) {
    CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X); CS(O);
    case '!': {
        auto [k, v] = UKV(std::move(x)).kv();
        return at(std::move(k), group(std::move(v)));
    }
    case '+': {
        auto [k, v] = UKV(+std::move(x)).kv();
        auto [gk, gv] = group_(L<O>(+std::move(v))).kv();
        return UKV(+UKV(std::move(k), +std::move(gk)),
                   +UKV(L<S>{S(0)}  , enlist(std::move(gv))));
    }
    default : throw Exception("nyi: group");
    }
#undef CS
}
