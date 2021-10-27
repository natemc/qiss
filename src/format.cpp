#include <format.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

int width(char         ) { return 1; }
int width(uint8_t     x) { return 1 + (x > 10) + (x > 100); }
int width(int32_t     x) { return (x < 0) + width(uint32_t(std::abs(x))); }
int width(int64_t     x) { return (x < 0) + width(uint64_t(std::abs(x))); }
int width(double      x) { return snprintf(nullptr, 0, "%f", x); }
int width(uint32_t    x) { return int(floor(log10(std::max(1u, x)))) + 1; }
int width(uint64_t    x) { return int(floor(log10(double(std::max(uint64_t(1), x))))) + 1; }
#ifdef __clang__
int width(std::size_t x) { return int(floor(log10(std::max(std::size_t(1), x)))) + 1; }
#endif
int width(const void* x) { return snprintf(nullptr, 0, "%p", x); }
int width(const char* x) { return static_cast<int>(strlen(x)); }
int width(B  ) { return 1; }
int width(C x) { return width(C::rep(x)); }
int width(D  ) { return 10; }
int width(F x) { return width(F::rep(x)); }
int width(H x) { return width(H::rep(x)); }
int width(I x) { return width(I::rep(x)); }
int width(J x) { return width(J::rep(x)); }
int width(S x) { return width(S::rep(x)); }
int width(T  ) { return 12; }
int width(X  ) { return 2; }
