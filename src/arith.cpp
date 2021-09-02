#include <arith.h>
#include <atomic.h>

ATOMIC_BEGIN_SIMPLE(Add, +)
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,D); ATOMIC_BOTH_WAYS(B,F);
                ATOMIC_BOTH_WAYS(B,I);
                ATOMIC_BOTH_WAYS(B,J); ATOMIC_BOTH_WAYS(B,T);
    ATOMICO(D); ATOMIC_BOTH_WAYS(D,I); ATOMIC_BOTH_WAYS(D,J);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J); ATOMIC_BOTH_WAYS(I,T);
    ATOMIC1(J); ATOMIC_BOTH_WAYS(J,T);
    ATOMICO(T);
    ATOMICOO();
ATOMIC_END(add)

ATOMIC_BEGIN_SIMPLE(Div, /)
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J);
    ATOMICOO();
ATOMIC_END(div)

ATOMIC_BEGIN_SIMPLE(Mod, %)
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J);
    ATOMICOO();
ATOMIC_END(mod)

ATOMIC_BEGIN(FDiv, %, F(F::rep(typename X::rep(x))) / F(F::rep(typename Y::rep(y))))
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J);
    ATOMICOO();
ATOMIC_END(fdiv)

ATOMIC_BEGIN_SIMPLE(Mul, *)
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,F); ATOMIC_BOTH_WAYS(B,I);
                ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J);
    ATOMICOO();
ATOMIC_END(mul)

ATOMIC_BEGIN_SIMPLE(Sub, -)
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,F); ATOMIC_BOTH_WAYS(B,I);
                ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(D); ATOMIC2(D,B); ATOMIC2(D,I); ATOMIC2(D,J);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J);
    ATOMIC1(T); ATOMIC2(T,B); ATOMIC2(T,I); ATOMIC2(T,J);
    ATOMICOO();
ATOMIC_END(sub)
