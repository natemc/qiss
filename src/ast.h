#pragma once

#include <cstdint>
#include <prim.h>

enum class Ast: char {
    adverb  = 'a',
    apply   = 'A',
    bind    = ':',
    cond    = '$',
    export_ = '+',
    formals = 'f',
    hole    = 'h',
    id      = 'i',
    infix   = 'I',
    list    = 'e',
    juxt    = 'j',
    lambda  = 'l',
    lit     = 'L',
    module  = 'm',
    op      = 'o',
    ref     = 'r',
};

inline bool operator==(Ast x, C   y) { return C(C::rep(x)) == y; }
inline bool operator==(C x  , Ast y) { return y == x; }
inline bool operator!=(Ast x, C   y) { return !(x == y); }
inline bool operator!=(C x  , Ast y) { return y != x; }
