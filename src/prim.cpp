#include <prim.h>
#include <cmath>
#include <cstring>
#include <ctime>
#include <doctest.h>
#include <limits>
#include <sym.h>

namespace {
    constexpr int days_from_1970_to_2000 = 30 * 365 + 7;
    constexpr int seconds_per_day        = 24 * 60 * 60;

    D unix2date(time_t x) {
        return D(D::rep(x / seconds_per_day - days_from_1970_to_2000));
    }
} // unnamed

// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition
bool fuzzy_match(double x, double y) {
    constexpr int64_t ULPs = 4;
    union Pun {
        explicit Pun(double x): d(x) {}
        double  d;
        int64_t i;
    };
    return isnan(x) && isnan(y)
        || fabs(x - y) <= std::numeric_limits<double>::epsilon()
        || (0 < x) == (0 < y) && abs(Pun(x).i - Pun(y).i) <= ULPs;
}
TEST_CASE("fuzzy match should be much better tested") {
    CHECK(fuzzy_match(0, 0));

    double sum = 0;
    for (int i = 0; i < 10; ++i) sum += 0.1;
    CHECK(sum != 1.0);
    CHECK(fuzzy_match(sum, 1.0));
    CHECK(!fuzzy_match(sum, 1.0000000000001));
    CHECK(!fuzzy_match(sum, 0.9999999999999));
}

T hmsm2time(int h, int m, int s, int millis) {
    return T(((h * 60 + m) * 60 + s) * 1000 + millis);
}

bool symless(S::rep x, S::rep y) {
    // TODO use sym_rank instead
    return strcmp(c_str(S(x)), c_str(S(y))) < 0;
}

D ymd2date(int y, int m, int d) {
    tm ymd = { 0, 0, 0, d, m-1, y-1900, 0, 0, 0, 0, 0 };
    const time_t t = mktime(&ymd);
    return t == -1? ND : unix2date(y<1970? t-seconds_per_day : t);
}
TEST_CASE("ymd2date returns the zero D for 2000.01.01") {
    CHECK(D(0) == ymd2date(2000, 1, 1));
}
