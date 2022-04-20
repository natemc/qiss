#include <relop.h>
#include <atomic.h>
#include <type_traits>

namespace std {
    template <> struct common_type<B, I> { using type = I; };
    template <> struct common_type<I, B> { using type = I; };
    template <> struct common_type<B, J> { using type = J; };
    template <> struct common_type<J, B> { using type = J; };
    template <> struct common_type<F, I> { using type = F; };
    template <> struct common_type<I, F> { using type = F; };
    template <> struct common_type<F, J> { using type = F; };
    template <> struct common_type<J, F> { using type = F; };
    template <> struct common_type<I, J> { using type = J; };
    template <> struct common_type<J, I> { using type = J; };
}

ATOMIC_BEGIN(Eq, =, B(x == y))
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,I); ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(C); ATOMIC1(D);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(H);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J); ATOMIC1(S); ATOMIC1(T); ATOMIC1(X);
    ATOMICOO();
ATOMIC_END(eq)

ATOMIC_BEGIN(Less, <, B(x < y))
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,I); ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(C); ATOMIC1(D);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(H);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J); ATOMIC1(S); ATOMIC1(T); ATOMIC1(X);
    ATOMICOO();
ATOMIC_END(less)

ATOMIC_BEGIN(Greater, >, B(x > y))
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,I); ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(C); ATOMIC1(D);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(H);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J); ATOMIC1(S); ATOMIC1(T); ATOMIC1(X);
    ATOMICOO();
ATOMIC_END(greater)

ATOMIC_BEGIN(Max, |, (x < y? std::common_type_t<decltype(x), decltype(y)>(y)
                           : std::common_type_t<decltype(x), decltype(y)>(x))) // cppcheck-suppress syntaxError
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,I); ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(C); ATOMIC1(D);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(H);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J); ATOMIC1(S); ATOMIC1(T); ATOMIC1(X);
    ATOMICOO();
ATOMIC_END(max)

ATOMIC_BEGIN(Min, |, (x < y? std::common_type_t<decltype(x), decltype(y)>(x)
                           : std::common_type_t<decltype(x), decltype(y)>(y))) // cppcheck-suppress syntaxError
    ATOMIC1(B); ATOMIC_BOTH_WAYS(B,I); ATOMIC_BOTH_WAYS(B,J);
    ATOMIC1(C); ATOMIC1(D);
    ATOMIC1(F); ATOMIC_BOTH_WAYS(F,I); ATOMIC_BOTH_WAYS(F,J);
    ATOMIC1(H);
    ATOMIC1(I); ATOMIC_BOTH_WAYS(I,J);
    ATOMIC1(J); ATOMIC1(S); ATOMIC1(T); ATOMIC1(X);
    ATOMICOO();
ATOMIC_END(min)
