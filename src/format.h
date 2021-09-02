#pragma once

#include <algorithm>
#include <cstdint>
#include <prim.h>

int width(char);
int width(uint8_t);
int width(int32_t);
int width(int64_t);
int width(double);
int width(uint32_t);
int width(uint64_t);
int width(std::size_t);
int width(const void*);
int width(const char*);

int width(B x);
int width(C x);
int width(D x);
int width(F x);
int width(H x);
int width(I x);
int width(J x);
int width(S x);
int width(T x);
int width(X x);

template <class X> struct Left {
    Left(int w_, const X* x_, char f_=' '): w(w_), x(x_), f(f_) {}
    int      w;
    const X* x;
    char     f;
};

template <class X> Left<X> left(int width, const X& x, char fill=' ') {
    return Left<X>(width, &x, fill);
}

template <class X> Left<X> left(J width, const X& x, char fill=' ') {
    return Left<X>(int(J::rep(width)), &x, fill);
}

template <class X> struct Right {
    Right(int w_, const X* x_, char f_=' '): w(w_), x(x_), f(f_) {}
    int      w;
    const X* x;
    char     f;
};

template <class X> Right<X> right(int width, const X& x, char fill=' ') {
    return Right<X>(width, &x, fill);
}

template <class X> Right<X> right(J width, const X& x, char fill=' ') {
    return Right<X>(J::rep(width), &x, fill);
}

template <class STREAM, class X>
decltype(auto) operator<<(STREAM&& s, const Left<X>& x) {
    const int p = x.w - std::min(x.w, int(width(*x.x)));
    s << *x.x;
    for (int i = 0; i < p; ++i) s << x.f;
    return std::forward<STREAM>(s);
}

template <class STREAM, class X>
decltype(auto) operator<<(STREAM&& s, const Right<X>& x) {
    const int p = x.w - std::min(x.w, int(width(*x.x)));
    for (int i = 0; i < p; ++i) s << x.f;
    return std::forward<STREAM>(s) << *x.x;
}
