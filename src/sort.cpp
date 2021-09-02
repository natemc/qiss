#include <sort.h>
#include <exception.h>

namespace {
template <class X> using OT = ObjectTraits<X>;
}

bool is_less(O x, O y) {
    // k allows this (??) e.g., <(1;`x;3.1) => 0 2 1
    if (x.type() != y.type()) throw Exception("type: < (iasc)");
#define CS(X) case -OT<X>::typei(): return x.atom<X>() < y.atom<X>(); \
              case OT<X>::typei() : return is_less_(L<X>(std::move(x)), L<X>(std::move(y)))
    switch (int(x.type())) {
    CS(B); CS(C); CS(D); CS(F); CS(J); CS(S); CS(T); CS(X);
    case 0  : {
        L<O> xs(std::move(x));
        L<O> ys(std::move(y));
        return std::lexicographical_compare(
            xs.begin(), xs.end(), ys.begin(), ys.end(), is_less);
        }
    default : throw Exception("nyi: sort list of tables??");
    }
#undef CS
}
