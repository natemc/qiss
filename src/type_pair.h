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
