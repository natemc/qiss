#pragma once

#include <cstdint>
#include <prim.h>

enum class Ast: uint8_t {
    adverb  = 'A',
    apply   = 'a',
    bind    = ':',
    cond    = '$',
    export_ = '+',
    exprs   = 'E',
    formals = 'f',
    hole    = 'h',
    id      = 'I',
    infix   = 'i',
    list    = 'L',
    juxt    = 'J',
    lambda  = 'l',
    lit     = 'n',
    module  = 'M',
    op      = 'O',
    ref     = 'r',
    ret     = 'R',
    stmts   = 'S',
};

inline bool operator==(Ast x, X   y) { return X(X::rep(x)) == y; }
inline bool operator==(X x  , Ast y) { return y == x; }
inline bool operator!=(Ast x, X   y) { return !(x == y); }
inline bool operator!=(X x  , Ast y) { return y != x; }
