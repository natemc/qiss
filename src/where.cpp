#include <where.h>
#include <at.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <object.h>
#include <sum.h>
#include <ukv.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class R> L<R> where_(const L<B>& x) {
        L<R> r;
        r.reserve(sum(x));
        for (typename R::rep i(0); i<x.size(); ++i)
            if (x[i]) r.emplace_back(i);
        return r;
    }

    template <class R, class X> L<R> where_(const L<X>& x) {
        L<R> r;
        r.reserve(sum(x));
        for (typename R::rep i(0); i<x.size(); ++i)
            for (index_t k(0); k < typename X::rep(x[i]); ++k)
                r.emplace_back(i);
        return r;
    }
}

O where(O x) {
    switch (int(x.type())) {
    case OT<B>::typei(): return where_<J>(L<B>(std::move(x)));
    case OT<I>::typei(): return where_<J>(L<I>(std::move(x)));
    case OT<J>::typei(): return where_<J>(L<J>(std::move(x)));
    case '!': {
        auto [k, v] = UKV(std::move(x)).kv();
        return at(std::move(k), where(std::move(v)));
    }
    default : throw Exception("'type (where)");
    }
}

O wherei(O x) {
    switch (int(x.type())) {
    case OT<B>::typei(): return where_<I>(L<B>(std::move(x)));
    case OT<I>::typei(): return where_<I>(L<I>(std::move(x)));
    case OT<J>::typei(): return where_<I>(L<J>(std::move(x)));
    case '!': {
        auto [k, v] = UKV(std::move(x)).kv();
        return at(std::move(k), wherei(std::move(v)));
    }
    default : throw Exception("'type (where)");
    }
}
