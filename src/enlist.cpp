#include <enlist.h>
#include <each.h>
#include <l.h>
#include <o.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Enlist {
        template <class X> L<X> operator()(X x) const { return L<X>{x}; }

        O operator()(O x) const {
#define CS(X) case -OT<X>::typei(): return (*this)(x.atom<X>()); \
              case OT<X>::typei() : return L<O>{x}
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I);
            CS(J); CS(S); CS(T); CS(X);
            case OT<O>::typei(): return L<O>{x};
            case '!': return O(make_table(addref(dk(x.get())),
                                          each(*this, O(addref(dv(x.get())))).release()));
            case '+': throw Exception("nyi: , (enlist) on table");
            default: throw Exception("nyi: , (enlist)");
            }
#undef CS
        }
    } enlist_;
}

O enlist(O x) { return enlist_(std::move(x)); }
