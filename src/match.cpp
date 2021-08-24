#include <match.h>
#include <exception.h>
#include <o.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    bool match_dict(UKV x, UKV y) {
        return match_(x.key(), y.key()) && match_(x.val(), y.val());
    }
}

bool Match_::operator()(O x, O y) const {
    if (x.get () == y.get ()) return true;
    if (x.type() != y.type()) return false;

#define ATOM(X) case -OT<X>::typei(): return x.atom<X>() == y.atom<X>();
#define LIST(X) case  OT<X>::typei(): return (*this)(L<X>(std::move(x)), L<X>(std::move(y)));
#define BOTH(X) ATOM(X); LIST(X)
    switch (int(x.type())) {
    BOTH(B); BOTH(C); BOTH(D); BOTH(F); BOTH(H); BOTH(I);
    BOTH(J); BOTH(S); BOTH(T); BOTH(X);
    LIST(O);
    case '!': return match_dict(UKV(std::move(x)), UKV(std::move(y)));
    case '+': return match_dict(UKV(addref(x.get()->dict)), UKV(addref(y.get()->dict)));
    default : throw Exception("nyi: match");
    }
#undef ATOM
#undef LIST
}

O match(O x, O y) {
    return O(B(match_(x, y)));
}
