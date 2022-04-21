#pragma once

#include <l.h>
#include <lambda.h>
#include <o.h>
#include <ukv.h>

template <class X, class Y> struct TypePair {
    template <class Z> using OT = ObjectTraits<Z>;
    constexpr static int AA = ((-OT<X>::typei() & 0xff) << 8) | (-OT<Y>::typei() & 0xff);
    constexpr static int AL = ((-OT<X>::typei() & 0xff) << 8) | OT<Y>::typei()          ;
    constexpr static int LA = (OT<X>::typei() << 8)           | (-OT<Y>::typei() & 0xff);
    constexpr static int LL = (OT<X>::typei() << 8)           | OT<Y>::typei()          ;
};

template <class X> struct TypePairDict {
    template <class Z> using OT = ObjectTraits<Z>;
    constexpr static int AD = ((-OT<X>::typei() & 0xff) << 8) | '!';
    constexpr static int DA = ('!' << 8)                      | (-OT<X>::typei() & 0xff);
    constexpr static int DL = ('!' << 8)                      | OT<X>::typei();
    constexpr static int LD = (OT<X>::typei() << 8)           | '!';
};

template <class X> struct TypePairTable {
    template <class Z> using OT = ObjectTraits<Z>;
    constexpr static int AT = ((-OT<X>::typei() & 0xff) << 8) | '+';
    constexpr static int TA = ('+' << 8)                      | (-OT<X>::typei() & 0xff);
    constexpr static int TL = ('+' << 8)                      | OT<X>::typei();
    constexpr static int LT = (OT<X>::typei() << 8)           | '+';
};

inline constexpr int type_pair(Type x, Type y) {
    return ((int(x) & 0xff) << 8) | (int(y) & 0xff);
}

inline int type_pair(const O& x, const O& y) {
    return type_pair(x.type(), y.type());
}

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
