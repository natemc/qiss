#include <output.h>
#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <primio.h>

namespace {
    time_t date2unix(D x) { return 86400 * (D::rep(x) + 10957); }

    constexpr char digit[] = "0123456789abcdef";

    template <std::size_t N> std::size_t rite(char* buf, const char (&a)[N]) {
        memcpy(buf, a, N - 1);
        return N - 1;
    }
}

std::size_t escapec::write(char* buf, C x) {
    if (!strchr("\\\"\n\r\t", C::rep(x))) {
        buf[0] = C::rep(x);
        return 1;
    }
    buf[0] = '\\';
    switch (C::rep(x)) {
    case '\n': buf[1] = 'n'; break;
    case '\r': buf[1] = 'r'; break;
    case '\t': buf[1] = 't'; break;
    default  : buf[1] = C::rep(x); break;
    }
    return 2;
}

std::size_t write_byte::write(char* buf, X x) {
    buf[0] = digit[X::rep(x) / 16];
    buf[1] = digit[X::rep(x) % 16];
    return 2;
}

std::size_t write_date::write(char* buf, D x) {
    if (x.is_null()) return rite(buf, "0Nd");
    // Do these even make sense? Have any use?
    if (x == WD) return rite(buf, "0Wd");
//    if (x == -WD) return rite(buf, "-0Wd");

    // TODO? k works for dates before 1900, but this does NOT.
    time_t t = date2unix(x);
    const tm * const ymd = gmtime(&t);
    const int y = 1900 + ymd->tm_year;
    const int m = ymd->tm_mon + 1;
    const int d = ymd->tm_mday;
    const char arr[] = {
        digit[y / 1000], digit[y / 100 % 10], digit[y / 10 % 10], digit[y % 10],
        '.', digit[m / 10], digit[m % 10], '.', digit[d / 10], digit[d % 10]
    };
    memcpy(buf, arr, sizeof arr);
    return sizeof arr;
}

std::size_t write_double::write(char* buf, double x) {
    if      (isnan(x)) return rite(buf, "0n");
    else if (isinf(x)) return x < 0? rite(buf, "-0w") : rite(buf, "0w");
//    std::to_chars(buf, buf + 64, x, std::chars_format::general);
    const int rc = snprintf(buf, 64, "%lg", x);
    assert(0 < rc);
    return std::size_t(rc);
}

std::size_t write_pointer::write(char* buf, const void* x) {
    X arr[sizeof x];
    memcpy(arr, &x, sizeof x);
    memcpy(buf, "0x", 2);
    for (std::size_t i = 0; i < sizeof x; ++i) {
        const X::rep v = X::rep(arr[sizeof x - i - 1]);
        buf[2*i + 2]   = digit[v / 16];
        buf[2*i + 3]   = digit[v % 16];
    }
    return 2 + sizeof(void*) * 2;
}

std::size_t write_time::write(char* buf, T x) {
    if (x.is_null()) return rite(buf, "0Nt");
    if (x == WT) return rite(buf, "0Wt");
    if (x == -WT) return rite(buf, "-0Wt");
    const T::rep r(x);
    const int h  = r / 3600000;
    assert(0 <= h && h <= 99);
    const int m  = r / 60000 % 60;
    const int s  = r / 1000 % 60;
    const int ms = r % 1000;
    const char arr[] = {digit[h / 10], digit[h % 10], ':',
                        digit[m / 10], digit[m % 10], ':',
                        digit[s / 10], digit[s % 10], '.',
                        digit[ms / 100], digit[ms / 10 % 10], digit[ms % 10]};
    memcpy(buf, arr, sizeof arr);
    return sizeof arr;
}

std::size_t write_uint64::write(char* buf, uint64_t x) {
    // TODO use std::to_chars when it becomes available
    // const auto [p, ec] = std::to_chars(buf, buf + sizeof buf, x);

    if (x == 0) return rite(buf, "0");
    std::size_t i = 0;
    for (uint64_t p = x; p; p /= 10, ++i) buf[i] = '0' + p % 10;
    std::reverse(buf, buf + i);
    return i;
}
