#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

template <class X, class Y> X bitcast(Y y) {
    static_assert(sizeof(X) == sizeof(Y));
    X x;
    memcpy(&x, &y, sizeof x);
    return x;
}

// count leading zeroes (bits)
inline int clz(uint64_t x) {
    // __builtin_clzll(0) is 0 !! :-/
    return x == 0? 64 : __builtin_clzll(x);
}

// count trailing zeroes (bits)
inline int ctz(uint64_t x) {
    // __builtin_ctzll(0) is 0 !! :-/
    return x == 0? 64 : __builtin_ctzll(x);
}

constexpr inline bool is_power_of_2(uint64_t x) {
    return __builtin_popcountll(x) == 1;
}

inline int log2u64(uint64_t x) {
    assert(x);
    return 63 - clz(x);
}

inline uint64_t round_up_to_power_of_2(uint64_t x) {
    return x<=1ull? 1ull : 1ull << (64 - clz(x-1));
}
