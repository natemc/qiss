#pragma once

#include <cstdint>
#include <prim.h>
#include <utility>

#define WRITER(NAME, X, BUF_SZ) const struct NAME {                  \
    template <class S> decltype(auto) operator()(S&& s, X x) const { \
        char buf[BUF_SZ];                                            \
        return s << std::pair(buf, buf + write(buf, x));             \
    }                                                                \
private:                                                             \
    static std::size_t write(char* buf, X x);                        \
} NAME

WRITER(escapec      , C             , 2);
WRITER(write_byte   , X             , 2);
WRITER(write_date   , D             , 10);
WRITER(write_double , double        , 64);
WRITER(write_pointer, const void*   , 2 + sizeof(void*) * 2);
WRITER(write_time   , T             , 12);
WRITER(write_uint64 , uint64_t      , 32);

template <class STREAM> decltype(auto) write_int64(STREAM&& s, int64_t x) {
    if (x < 0) s << '-';
    return write_uint64(s, uint64_t(x < 0? -x : x));
}
