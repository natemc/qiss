#include <flip.h>
#include <algorithm>
#include <each.h>
#include <exception.h>
#include <l.h>
#include <o.h>
#include <primio.h>
#include <type_traits>
#include <ukv.h>
#include <ul.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    bool is_rectangular(L<O> x) {
        if (x.empty()) return true;
        if (!x[0].is_list()) return false;
        UL x0(x[0]);
        return std::all_of(x.begin(), x.end(), [&](O e) {
            return e.is_list() && UL(std::move(e)).size() == x0.size();
        });
    }

    bool is_uniform(L<O> x) {
        return std::all_of(x.begin(), x.end(),
            [=](O e){ return e.type() == x[0].type(); });
    }

    O ith(O x, index_t i) {
#define CS(X) case OT<X>::typei(): return O(L<X>(std::move(x))[i])
        switch (int(x->type)) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
        case OT<O>::typei(): return L<O>(std::move(x))[i];
        default : throw Exception("nyi: flip (+) for type");
        }
#undef CS
    }

    O dict2table(UKV x) {
        if (x.key().type() != OT<S>::typet())
            throw Exception("To make table from dict, key must be sym");
        if (x.val().type() != OT<O>::typet() || !is_rectangular(L<O>(x.val())))
            throw Exception("To make table from dict, value must be rectangular");
        return O(make_table(x.release()));
    }

    template <class X>
    L<O> transpose_uniform(L<O> x) {
        const index_t m = UL(x[0]).size();
        L<O> r;
        r.reserve(m);
        for (index_t i = 0; i < m; ++i)
            r.emplace_back(each([=](auto&& j){ return L<X>(j)[i]; }, x));
        return r;
    }

    L<O> uniform_transpose(L<O> x) {
#define CS(X) case OT<X>::typei(): return transpose_uniform<X>(std::move(x))
        switch (int(x[0]->type)) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X); CS(O);
        default : throw Exception("nyi: flip (+) for type");
        }
#undef CS
    }

    L<O> transpose(L<O> x) {
        if (x.empty()) return x;
        if (!is_rectangular(x))
            throw Exception("length: flip (+) arg must be rectangular");
        if (is_uniform(x)) return uniform_transpose(std::move(x));

        const index_t m = UL(x[0]).size();
        L<O> r;
        r.reserve(m);
        for (index_t i = 0; i < m; ++i)
            r.emplace_back(each([=](auto&& j){ return ith(j, i); }, x));
        return r;
    }
} // unnamed

O flip(O x) {
    switch (int(x.type())) {
    case 0  : return transpose(L<O>(std::move(x)));
    case '!': return dict2table(UKV(std::move(x)));
    case '+': return UKV(addref(x->dict));
    default : throw Exception("type: + (flip) requires matrix, dict, or table");
    }
}

O operator+(O x) { return flip(std::move(x)); }
