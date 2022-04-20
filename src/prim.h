#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

// Separate the type system of the language being defined from the type system
// of the implementation language and reduce unwanted implicit conversions.

template <class X> struct is_prim: std::false_type {};
template <class X> inline constexpr bool is_prim_v = is_prim<X>::value;

#define PRIM_BEGIN(TYPE, REP, EQTEST, LTTEST, NULLTEST) struct TYPE { \
  using rep = REP;                                                    \
  TYPE() = default;                                                   \
  explicit constexpr TYPE(rep x_): x(x_) {}                           \
  explicit constexpr operator const rep&() const { return x; }        \
  explicit constexpr operator bool      () const { return bool(x); }  \
  bool operator< (TYPE y) const { return LTTEST; }                    \
  bool operator==(TYPE y) const { return EQTEST; }                    \
  bool is_null() const { return NULLTEST; }

#define PRIM_END(TYPE)                                                \
private:                                                              \
  rep x;                                                              \
};                                                                    \
template <> struct is_prim<TYPE>: std::true_type {};                  \
inline bool operator> (TYPE x, TYPE y) { return y < x; }              \
inline bool operator<=(TYPE x, TYPE y) { return !(y < x); }           \
inline bool operator>=(TYPE x, TYPE y) { return !(x < y); }           \
inline bool operator!=(TYPE x, TYPE y) { return !(x == y); }

#define PRIM(TYPE, REP, EQTEST, LTTEST, NULLTEST) \
PRIM_BEGIN(TYPE, REP, EQTEST, LTTEST, NULLTEST) PRIM_END(TYPE)

#define PRIM_EQ_LT(TYPE, REP, NULLTEST) \
PRIM(TYPE, REP, x == y.x, x < y.x, NULLTEST)

#define NEVER_NULL false
PRIM_EQ_LT(B, uint8_t, NEVER_NULL)
PRIM_EQ_LT(C, char   , x == ' '  )
PRIM_EQ_LT(H, int    , NEVER_NULL)
constexpr H NH = H(std::numeric_limits<H::rep>::min());
PRIM_EQ_LT(X, uint8_t, NEVER_NULL)
#undef NEVER_NULL

PRIM_EQ_LT(D, int32_t, x == std::numeric_limits<rep>::min())
constexpr D ND = D(std::numeric_limits<D::rep>::min());
constexpr D WD = D(std::numeric_limits<D::rep>::max());
D ymd2date(int y, int m, int d); // DOES NO CHECKING!

PRIM_BEGIN(I, int32_t, x == y.x, x < y.x, x == std::numeric_limits<rep>::min())
    explicit I(B o): x(B::rep(o)) {}
    explicit I(X o): x(X::rep(o)) {}
PRIM_END(I)
constexpr I NI = I(std::numeric_limits<I::rep>::min());
constexpr I WI = I(std::numeric_limits<I::rep>::max());
inline I& operator+=(I& x, B y) { x = I(I::rep(x) + B::rep(y)); return x; }
inline I& operator+=(I& x, I y) { x = I(I::rep(x) + I::rep(y)); return x; }
inline I& operator-=(I& x, I y) { x = I(I::rep(x) - I::rep(y)); return x; }
inline I& operator-=(I& x, B y) { return x -= I(I::rep(B::rep(y))); }
inline I& operator++(I& x) { x = I(I::rep(x) + 1); return x; }
inline I operator++(I& x, int) { I old(x); ++x; return old; }
inline I operator-(I x) { return I(-I::rep(x)); }
inline I operator*(I x, I y) { return I(I::rep(x) * I::rep(y)); }
inline I operator-(I x, I y) { return I(I::rep(x) - I::rep(y)); }
inline I operator+(I x, I y) { return I(I::rep(x) + I::rep(y)); }
inline I operator/(I x, I y) { return I::rep(y)? I(I::rep(x) / I::rep(y)) : NI; }
inline I operator%(I x, I y) { return I::rep(y)? I(I::rep(x) % I::rep(y)) : NI; }

PRIM_BEGIN(J, int64_t, x == y.x, x < y.x, x == std::numeric_limits<rep>::min())
    explicit J(B o): x(B::rep(o)) {}
    explicit J(I o): x(I::rep(o)) {}
    explicit J(X o): x(X::rep(o)) {}
PRIM_END(J)
constexpr J NJ = J(std::numeric_limits<J::rep>::min());
constexpr J WJ = J(std::numeric_limits<J::rep>::max());
inline J& operator+=(J& x, J y) { x = J(J::rep(x) + J::rep(y)); return x; }
inline J& operator+=(J& x, B y) { return x += J(J::rep(B::rep(y))); }
inline J& operator-=(J& x, J y) { x = J(J::rep(x) - J::rep(y)); return x; }
inline J& operator-=(J& x, B y) { return x -= J(J::rep(B::rep(y))); }
inline J& operator++(J& x) { x = J(J::rep(x) + 1); return x; }
inline J operator++(J& x, int) { J old(x); ++x; return old; }
inline J& operator--(J& x) { x = J(J::rep(x) - 1); return x; }
inline J operator--(J& x, int) { J old(x); --x; return old; }
inline J operator-(J x) { return J(-J::rep(x)); }
inline J operator*(J x, J y) { return J(J::rep(x) * J::rep(y)); }
inline J operator-(J x, J y) { return J(J::rep(x) - J::rep(y)); }
inline J operator+(J x, J y) { return J(J::rep(x) + J::rep(y)); }
inline J operator/(J x, J y) { return J::rep(y)? J(J::rep(x) / J::rep(y)) : NJ; }
inline J operator%(J x, J y) { return J::rep(y)? J(J::rep(x) % J::rep(y)) : NJ; }

bool fuzzy_match(double x, double y);
PRIM_BEGIN(F, double, fuzzy_match(x, y.x), x < y.x, std::isnan(x))
    explicit F(B o): x(B::rep(o)) {}
    explicit F(I o): x(I::rep(o)) {}
    explicit F(J o): x(J::rep(o)) {}
    explicit F(X o): x(X::rep(o)) {}
PRIM_END(F)
constexpr F NF = F(std::numeric_limits<F::rep>::quiet_NaN());
constexpr F WF = F(std::numeric_limits<F::rep>::infinity());
inline F operator-(F x) { return F(-F::rep(x)); }
inline F operator*(F x, F y) { return F(F::rep(x) * F::rep(y)); }
inline F operator-(F x, F y) { return F(F::rep(x) - F::rep(y)); }
inline F operator+(F x, F y) { return F(F::rep(x) + F::rep(y)); }
inline F operator/(F x, F y) { return F(F::rep(x) / F::rep(y)); }

bool symless(int32_t, int32_t);
PRIM(S, int32_t, x == y.x, symless(x, y.x), x == 0)
constexpr S NS = S(0);

PRIM_EQ_LT(T, int32_t, x == std::numeric_limits<rep>::min())
constexpr T NT = T(std::numeric_limits<T::rep>::min());
constexpr T WT = T(std::numeric_limits<T::rep>::max());
inline T operator-(T x) { return T(-T::rep(x)); }
inline J operator-(T x, T y) { return J(T::rep(x) - T::rep(y)); }
inline T operator+(T x, I y) { return T(T::rep(x) + I::rep(y)); }
inline T operator+(T x, J y) { return T(T::rep(T::rep(x) + J::rep(y))); }
T hmsm2time(int h, int m, int s, int millis);

inline J operator+(B x, B y) { return J(B::rep(x)) + J(B::rep(y)); }
inline J operator-(B x, B y) { return J(B::rep(x)) - J(B::rep(y)); }
inline J operator*(B x, B y) { return J(B::rep(x)) * J(B::rep(y)); }
inline J operator/(B x, B y) { return J(B::rep(x)) / J(B::rep(y)); }

inline F operator+(B x, F y) { return F(B::rep(x)) + y; }
inline F operator-(B x, F y) { return F(B::rep(x)) - y; }
inline F operator*(B x, F y) { return F(B::rep(x)) * y; }
inline F operator/(B x, F y) { return F(B::rep(x)) / y; }

inline F operator+(F x, B y) { return x + F(B::rep(y)); }
inline F operator-(F x, B y) { return x - F(B::rep(y)); }
inline F operator*(F x, B y) { return x * F(B::rep(y)); }
inline F operator/(F x, B y) { return x / F(B::rep(y)); }

inline D operator+(D x, B y) { return D(D::rep(x) + B::rep(y)); }
inline D operator+(D x, I y) { return D(D::rep(x) + I::rep(y)); }
inline D operator+(D x, J y) { return D(D::rep(D::rep(x) + J::rep(y))); }
inline D operator+(B x, D y) { return D(B::rep(x) + D::rep(y)); }
inline D operator+(I x, D y) { return D(I::rep(x) + D::rep(y)); }
inline D operator+(J x, D y) { return D(D::rep(J::rep(x) + D::rep(y))); }
inline D operator-(D x, B y) { return D(D::rep(x) - B::rep(y)); }
inline D operator-(D x, I y) { return D(D::rep(x) - I::rep(y)); }
inline D operator-(D x, J y) { return D(D::rep(D::rep(x) - J::rep(y))); }
inline J operator-(D x, D y) { return J(D::rep(x) - D::rep(y)); }

inline T operator+(T x, B y) { return T(T::rep(x) + B::rep(y)); }
inline T operator+(B x, T y) { return T(B::rep(x) + T::rep(y)); }
inline T operator+(I x, T y) { return T(I::rep(x) + T::rep(y)); }
inline T operator+(J x, T y) { return T(T::rep(J::rep(x) + T::rep(y))); }
inline T operator-(T x, B y) { return T(T::rep(x) - B::rep(y)); }
inline T operator-(T x, I y) { return T(T::rep(x) - I::rep(y)); }
inline T operator-(T x, J y) { return T(T::rep(T::rep(x) - J::rep(y))); }

inline F operator+(I x, F y) { return F(I::rep(x)) + y; }
inline F operator-(I x, F y) { return F(I::rep(x)) - y; }
inline F operator*(I x, F y) { return F(I::rep(x)) * y; }
inline F operator/(I x, F y) { return F(I::rep(x)) / y; }
inline F operator+(J x, F y) { return F(F::rep(J::rep(x))) + y; }
inline F operator-(J x, F y) { return F(F::rep(J::rep(x))) - y; }
inline F operator*(J x, F y) { return F(F::rep(J::rep(x))) * y; }
inline F operator/(J x, F y) { return F(F::rep(J::rep(x))) / y; }

inline F operator+(F x, I y) { return x + F(I::rep(y)); }
inline F operator-(F x, I y) { return x - F(I::rep(y)); }
inline F operator*(F x, I y) { return x * F(I::rep(y)); }
inline F operator/(F x, I y) { return x / F(I::rep(y)); }
inline F operator+(F x, J y) { return x + F(F::rep(J::rep(y))); }
inline F operator-(F x, J y) { return x - F(F::rep(J::rep(y))); }
inline F operator*(F x, J y) { return x * F(F::rep(J::rep(y))); }
inline F operator/(F x, J y) { return x / F(F::rep(J::rep(y))); }

inline I operator+(B x, I y) { return I(B::rep(x)) + y; }
inline I operator-(B x, I y) { return I(B::rep(x)) - y; }
inline I operator*(B x, I y) { return I(B::rep(x)) * y; }
inline I operator/(B x, I y) { return I(B::rep(x)) / y; }
inline I operator%(B x, I y) { return I(B::rep(x)) % y; }
inline J operator+(B x, J y) { return J(B::rep(x)) + y; }
inline J operator-(B x, J y) { return J(B::rep(x)) - y; }
inline J operator*(B x, J y) { return J(B::rep(x)) * y; }
inline J operator/(B x, J y) { return J(B::rep(x)) / y; }
inline J operator%(B x, J y) { return J(B::rep(x)) % y; }

inline I operator+(I x, B y) { return x + I(B::rep(y)); }
inline I operator-(I x, B y) { return x - I(B::rep(y)); }
inline I operator*(I x, B y) { return x * I(B::rep(y)); }
inline I operator/(I x, B y) { return x / I(B::rep(y)); }
inline I operator%(I x, B y) { return x % I(B::rep(y)); }
inline J operator+(J x, B y) { return x + J(B::rep(y)); }
inline J operator-(J x, B y) { return x - J(B::rep(y)); }
inline J operator*(J x, B y) { return x * J(B::rep(y)); }
inline J operator/(J x, B y) { return x / J(B::rep(y)); }
inline J operator%(J x, B y) { return x % J(B::rep(y)); }

inline J operator+(I x, J y) { return J(I::rep(x)) + y; }
inline J operator-(I x, J y) { return J(I::rep(x)) - y; }
inline J operator*(I x, J y) { return J(I::rep(x)) * y; }
inline J operator/(I x, J y) { return J(I::rep(x)) / y; }
inline J operator%(I x, J y) { return J(I::rep(x)) % y; }
inline J operator+(J x, I y) { return x + J(I::rep(y)); }
inline J operator-(J x, I y) { return x - J(I::rep(y)); }
inline J operator*(J x, I y) { return x * J(I::rep(y)); }
inline J operator/(J x, I y) { return x / J(I::rep(y)); }
inline J operator%(J x, I y) { return x % J(I::rep(y)); }

inline X& operator+=(X& x, X y) { return x = X(X::rep(x) + B::rep(y)); }
inline X& operator+=(X& x, B y) { return x += X(B::rep(y)); }
inline X operator+(X x, int y) { return X(X::rep(X::rep(x) + y)); }
inline X operator-(X x, int y) { return X(X::rep(X::rep(x) - y)); }
inline X operator+(X x, X   y) { return X(X::rep(x) + X::rep(y)); }
inline X operator-(X x, X   y) { return X(X::rep(x) - X::rep(y)); }

inline B operator==(B x, I y) { return B(B::rep(x) == I::rep(y)); }
inline B operator==(B x, J y) { return B(B::rep(x) == J::rep(y)); }
inline B operator==(I x, B y) { return B(I::rep(x) == B::rep(y)); }
inline B operator==(J x, B y) { return B(J::rep(x) == B::rep(y)); }
inline B operator==(F x, I y) { return B(F::rep(x) == I::rep(y)); }
inline B operator==(I x, F y) { return B(I::rep(x) == F::rep(y)); }
inline B operator==(F x, J y) { return B(F::rep(x) == F::rep(J::rep(y))); }
inline B operator==(J x, F y) { return B(F::rep(J::rep(x)) == F::rep(y)); }
inline B operator==(I x, J y) { return B(I::rep(x) == J::rep(y)); }
inline B operator==(J x, I y) { return B(J::rep(x) == I::rep(y)); }

inline B operator<(B x, I y) { return B(B::rep(x) < I::rep(y)); }
inline B operator<(B x, J y) { return B(B::rep(x) < J::rep(y)); }
inline B operator<(I x, B y) { return B(I::rep(x) < B::rep(y)); }
inline B operator<(J x, B y) { return B(J::rep(x) < B::rep(y)); }
inline B operator<(F x, I y) { return B(F::rep(x) < I::rep(y)); }
inline B operator<(I x, F y) { return B(I::rep(x) < F::rep(y)); }
inline B operator<(F x, J y) { return B(F::rep(x) < F::rep(J::rep(y))); }
inline B operator<(J x, F y) { return B(F::rep(J::rep(x)) < F::rep(y)); }
inline B operator<(I x, J y) { return B(I::rep(x) < J::rep(y)); }
inline B operator<(J x, I y) { return B(J::rep(x) < I::rep(y)); }

inline B operator>(B x, I y) { return B(B::rep(x) > I::rep(y)); }
inline B operator>(B x, J y) { return B(B::rep(x) > J::rep(y)); }
inline B operator>(I x, B y) { return B(I::rep(x) > B::rep(y)); }
inline B operator>(J x, B y) { return B(J::rep(x) > B::rep(y)); }
inline B operator>(F x, I y) { return B(F::rep(x) > I::rep(y)); }
inline B operator>(I x, F y) { return B(I::rep(x) > F::rep(y)); }
inline B operator>(F x, J y) { return B(F::rep(x) > F::rep(J::rep(y))); }
inline B operator>(J x, F y) { return B(F::rep(J::rep(x)) > F::rep(y)); }
inline B operator>(I x, J y) { return B(I::rep(x) > J::rep(y)); }
inline B operator>(J x, I y) { return B(J::rep(x) > I::rep(y)); }
