#pragma once

#include <each.h>
#include <merge_dicts.h>
#include <l.h>
#include <o.h>
#include <objectio.h>
#include <type_pair.h>
#include <ukv.h>

// This is somewhat nasty, but it removes a tremendous amount of duplication.

const struct LLeach {
    template <class F> O operator()(F&& f, L<O> x, L<O> y) const {
        return each(std::forward<F>(f), std::move(x), std::move(y));
    }
    template <class F, class X, class Y> requires is_prim_v<X> && is_prim_v<Y>
    O operator()(F&& f, L<X> x, L<Y> y) const {
        return each(std::forward<F>(f), std::move(x), std::move(y));
    }
    template <class F, class X> requires is_prim_v<X> O operator()(F&& f, L<X> x, L<O> y) const {
        return list_any_list<X>(std::forward<F>(f), std::move(x), std::move(y));
    }
    template <class F, class Y> requires is_prim_v<Y> O operator()(F&& f, L<O> x, L<Y> y) const {
        return any_list_list<Y>(std::forward<F>(f), std::move(x), std::move(y));
    }
} lleach;

template <class Derived> struct Atomic {
    const Derived* This() const { return static_cast<const Derived*>(this); }

    O operator()(O x, O y) const {
        // TODO both x and y are tables
        if (!x.is_dict() || !y.is_dict())
            // The usual llf is each
            return (*This())(lleach, std::move(x), std::move(y));
        // If both x and y are dicts tho, llf is merge_dicts
        UKV dx(std::move(x));
        UKV dy(std::move(y));
        auto llf = [&](auto self, auto xv, auto yv) {
            return merge_dicts(self, dx.key(), std::move(xv), dy.key(), std::move(yv));
        };
        return (*This())(llf, dx.val(), dy.val());
    }
};

#define ATOMIC_BEGIN(NAME, op, code) const struct NAME: Atomic<NAME> { \
    using Atomic<NAME>::operator();                                    \
    template <class X, class Y> requires is_prim_v<X> && is_prim_v<Y>  \
    auto operator()(X x, Y y) const { return code; }                   \
    Exception type_error(O x, O y) const {                             \
        L<C> err;                                                      \
        err << "type (" #op "): " << x->type << '\t' << y->type;       \
        return lc2ex(err);                                             \
    }                                                                  \
    template <class LLF> O operator()(LLF llf, O x, O y) const {       \
        switch (type_pair(x, y)) {

#define ATOMIC_BEGIN_SIMPLE(NAME, op) ATOMIC_BEGIN(NAME, op, x op y) \

#define ATOMIC_END(FUN) default: throw type_error(x, y); }}} FUN##_; \
O FUN(O x, O y) { return FUN##_(std::move(x), std::move(y)); }

#define ATOMIC2(X,Y)                                                                 \
case TypePair<X,Y>::AA: return O   ((*this)(x.atom<X>()       , y.atom<Y>()));       \
case TypePair<X,Y>::AL: return eachR(*this, x.atom<X>()       , L<Y>(std::move(y))); \
case TypePair<X,Y>::LA: return eachL(*this, L<X>(std::move(x)), y.atom<Y>());        \
case TypePair<X,Y>::LL: return llf  (*this, L<X>(std::move(x)), L<Y>(std::move(y)))

#define ATOMICOO() case TypePair<O,O>::LL: \
    return llf(*this, L<O>(std::move(x)), L<O>(std::move(y)))

#define ATOMICO(X)                                                                   \
case TypePair<X,O>::AL: return eachR(*this, std::move(x), L<O>(std::move(y)));       \
case TypePair<X,O>::LL: return llf  (*this, L<X>(std::move(x)), L<O>(std::move(y))); \
case TypePair<O,X>::LA: return eachL(*this, L<O>(std::move(x)), std::move(y));       \
case TypePair<O,X>::LL: return llf  (*this, L<O>(std::move(x)), L<X>(std::move(y)))

#define ATOMICD(X)                                                                          \
case TypePairDict<X>::AD: return atom_dict<X>(*this, std::move(x)     , UKV(std::move(y))); \
case TypePairDict<X>::DA: return dict_atom<X>(*this, UKV(std::move(x)), std::move(y));      \
case TypePairDict<X>::DL: return dict_list<X>(*this, UKV(std::move(x)), std::move(y));      \
case TypePairDict<X>::LD: return list_dict<X>(*this, std::move(x)     , UKV(std::move(y)))

#define ATOMICT(X)                                                                  \
case TypePairTable<X>::AT: return atom_table<X>(*this, std::move(x), std::move(y)); \
case TypePairTable<X>::TA: return table_atom<X>(*this, std::move(x), std::move(y)); \
case TypePairTable<X>::TL: return table_list<X>(*this, std::move(x), std::move(y)); \
case TypePairTable<X>::LT: return list_table<X>(*this, std::move(x), std::move(y))

#define ATOMIC1(X) ATOMIC2(X,X); ATOMICO(X); ATOMICD(X); ATOMICT(X)

#define ATOMIC_BOTH_WAYS(X,Y) ATOMIC2(X,Y); ATOMIC2(Y,X)

template <class X, class Y, class F>
O atom_atom(F&& f, O x, O y) {
    return std::forward<F>(f)(x.atom<X>(), y.atom<Y>());
}

template <class X, class Y, class F>
O atom_list(F&& f, O x, O y) {
    return eachR(std::forward<F>(f), x.atom<X>(), L<Y>(std::move(y)));
}

template <class F>
O atom_any_list(F&& f, O x, O y) {
    return eachR(std::forward<F>(f), std::move(x), L<O>(std::move(y)));
}

template <class F>
O any_list_atom(F&& f, O x, O y) {
    return eachL(std::forward<F>(f), L<O>(std::move(x)), std::move(y));
}

template <class X, class Y, class F>
O list_atom(F&& f, O x, O y) {
    return eachL(std::forward<F>(f), L<O>(std::move(x)), y.atom<Y>());
}

template <class X, class Y, class F>
O list_list(F&& f, O x, O y) {
    return each(std::forward<F>(f), L<X>(std::move(x)), L<Y>(std::move(y)));
}

template <class X, class F>
O list_any_list(F&& f, O x, O y) {
    return each(L2(f(O(x), std::move(y))), L<X>(std::move(x)), L<O>(std::move(y)));
}

template <class Y, class F>
O any_list_list(F&& f, O x, O y) {
    return each(L2(f(std::move(x), O(y))), L<O>(std::move(x)), L<Y>(std::move(y)));
}

template <class X, class F>
UKV atom_dict(F&& f, O x, UKV y) {
    return UKV(y.key(), std::forward<F>(f)(std::move(x), y.val()));
}

template <class X, class F>
UKV dict_atom(F&& f, UKV x, O y) {
    return UKV(x.key(), std::forward<F>(f)(x.val(), std::move(y)));
}

template <class X, class F>
UKV dict_list(F&& f, UKV x, O y) {
    return UKV(x.key(), std::forward<F>(f)(x.val(), std::move(y)));
}

template <class X, class F>
UKV list_dict(F&& f, O x, UKV y) {
    return UKV(y.key(), std::forward<F>(f)(std::move(x), y.val()));
}

template <class X, class F>
O atom_table(F&& f, O x, O y) {
    UKV d(addref(y.get()->dict));
    UKV r(atom_dict<X>(std::forward<F>(f), std::move(x), std::move(d)));
    return O(make_table(r.release()));
}

template <class X, class F>
O table_atom(F&& f, O x, O y) {
    UKV d(addref(x.get()->dict));
    UKV r(dict_atom<X>(std::forward<F>(f), std::move(d), std::move(y)));
    return O(make_table(r.release()));
}

template <class X, class F>
O table_list(F&& f, O x, O y) {
    UKV d(addref(x.get()->dict));
    UKV r(dict_list<X>(std::forward<F>(f), std::move(d), std::move(y)));
    return O(make_table(r.release()));
}

template <class X, class F>
O list_table(F&& f, O x, O y) {
    UKV d(addref(y.get()->dict));
    UKV r(list_dict<X>(std::forward<F>(f), std::move(x), std::move(d)));
    return O(make_table(r.release()));
}
