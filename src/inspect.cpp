#include <inspect.h>
#include <cctype>
#include <lcutil.h>
#include <o.h>
#include <objectio.h>
#include <primio.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;
}

H inspect(H h, O x) {
    L<C> s;
    return h << inspect(s, std::move(x));
}

L<C>& inspect(L<C>& s, O x) {
    Object* const o = x.get();
    s << static_cast<const void*>(o)
      << " {r:" << o->r << ", type:" << o->type << ", ";
#define ATOM(X) case -OT<X>::typei(): s << x.atom<X>(); break
#define LIST(X) case OT<X>::typei(): s << "cap:" << list_capacity(OT<X>::list(o)) << ", n:" << o->n << ", " << x; break
    switch (int(x.type())) {
    ATOM(B); ATOM(C); ATOM(D); ATOM(F); ATOM(I); ATOM(J); ATOM(S); ATOM(T); ATOM(X);
    LIST(B); LIST(C); LIST(D); LIST(F); LIST(I); LIST(J); LIST(S); LIST(T); LIST(X);
    case OT<O>::typei():
        s  << "cap:" << list_capacity(OT<O>::list(o)) << ", n:" << o->n << ", ";
        inspect(s, ((*OT<O>::list(o))[0]));
        for (index_t i=1; i<o->n; ++i)
            inspect(s << ", ", ((*OT<O>::list(o))[i]));
        break;
    case '!': {
        UKV d(x);
        inspect(inspect(s << "k:", d.key()) << ", v:", d.val());
        break;
        }
    case '+':
        s << "dict:";
        inspect(s, O(addref(x->dict)));
        break;
    default: s << "nyi: inspect";
    }
    return s << '}';
}

O inspect(O x) {
    L<C> s;
    inspect(s, x);
    H(1) << s << '\n' << flush;
    return x;
}
