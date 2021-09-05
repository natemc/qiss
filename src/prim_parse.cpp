#include <prim_parse.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <limits>
#include <o.h>
#include <sym.h>

namespace {
    constexpr char digits[] = "0123456789abcdef";

    int from_digits(const C* first, const C* last) {
        assert(all_digits(first, last));
        int r = 0;
        for (; first != last; ++first) r = r * 10 + C::rep(*first) - '0';
        return r;
    }

    X::rep parse_digit(C c) {
        return X::rep(std::find(digits, std::end(digits), tolower(C::rep(c))) - digits);
    }

    constexpr int days_from_1970_to_2000 = 30 * 365 + 7;
    constexpr int seconds_per_day        = 24 * 60 * 60;

    D unix2date(time_t x) {
        return D(D::rep(x / seconds_per_day - days_from_1970_to_2000));
    }
}

bool all_digits(const C* first, const C* last) {
    return std::all_of(first, last, [](C x){ return isdigit(C::rep(x)); });
}

std::pair<bool, B> parse_bool::operator()(const C* first, const C* last) const {
    return first == last? std::pair(false, B(0))
        :                 std::pair(true , B(!strchr("0fFnN", C::rep(*first))));
}

std::pair<bool, X> parse_byte::operator()(const C* first, const C* last) const {
    if (first == last) return std::pair(false, X(0));

    if (first + 1 == last) {
        const X::rep d(parse_digit(*first));
        return d < sizeof digits? std::pair(true, X(d)) : std::pair(false, X(0));
    }

    if (first + 2 != last) return std::pair(false, X(0));
    const X::rep msd(parse_digit(*first));
    const X::rep lsd(parse_digit(*(first + 1)));
    return msd < sizeof digits && lsd < sizeof digits?
        std::pair(true, X(X::rep(msd*16 + lsd))) : std::pair(false, X(0));
}

std::pair<bool, C> parse_char::operator()(const C* first, const C* last) const {
    return first == last || first + 1 != last? std::pair(false, C(' '))
        :                                      std::pair(true , *first);
}

std::pair<bool, D> parse_date::operator()(const C* first, const C* last) const {
    if (first == last) return std::pair(false, ND);

    const C* const p = first;
    const C* const q = last - (*(last - 1) == C('d'));
    if (*p == C('0') && p + 2 == q && !isdigit(C::rep(p[1]))) {
        return p[1] == C('N')? std::pair(true , ND)
            :  p[1] == C('W')? std::pair(true , WD)
            :  /* else */      std::pair(false, ND);
    }

    if (p + 8 == q) {
        if (!all_digits(p, q)) return std::pair(false, ND);
        const int r   = from_digits(p, q);
        const int y   = r / 10000;
        const int m   = r / 100 % 100;
        const int d   = r % 100;
        tm        ymd = { 0, 0, 0, d, m-1, y-1900, 0, 0, 0 };
        const time_t t   = mktime(&ymd);
        return t == -1? std::pair(false, ND)
            :           std::pair(true, unix2date(y<1970? t-86400 : t));
    }

    if (p + 10 != q) return std::pair(false, ND);
    if (!all_digits(p + 0, p +  4) ||
        !all_digits(p + 5, p +  7) ||
        !all_digits(p + 8, p + 10))
        return std::pair(false, ND);

    const int    y   = from_digits(p + 0, p + 4);
    const int    m   = from_digits(p + 5, p + 7);
    const int    d   = from_digits(p + 8, p + 10);
    tm           ymd = { 0, 0, 0, d, m-1, y-1900, 0, 0, 0 };
    const time_t t   = mktime(&ymd);
    return t == -1? std::pair(false, ND)
        :           std::pair(true, unix2date(y<1970? t-86400 : t));
}

std::pair<bool, F> parse_float::operator()(const C* first, const C* last) const {
    if (first == last) return std::pair(false, NF);

    const C* const p = first + (*first      == C('-'));
    const C* const q = last  - (*(last - 1) == C('f'));
    if (*p == C('0') && p + 2 == q && !isdigit(C::rep(p[1]))) {
        const char c = char(tolower(C::rep(p[1])));
        return c == 'n'? std::pair(first == p, F(NF))
            :  c == 'w'? std::pair(true, first == p? WF : -WF)
            :  /*else*/  std::pair(false, NF);
    }

    L<C> s(q - p + 1);
    auto it = std::copy_if(p, q, s.begin(), [](C c){return c != C(',');});
    *it = C('\0');
    const C::rep* const begin = reinterpret_cast<C::rep*>(s.begin());
    char* end;
    const double f = std::strtod(begin, &end);
    return begin == end? std::pair(false, NF)
        :  /* else */    std::pair(true, F(first == p? f : -f));
       
}

std::pair<bool, J> parse_long::operator()(const C* first, const C* last) const {
    if (first == last) return std::pair(false, NJ);

    const C*       p = first + (*first      == C('-'));
    const C* const q = last  - (*(last - 1) == C('j'));
    if (*p == C('0') && p + 2 == q && !isdigit(C::rep(p[1]))) {
        return p[1] == C('N')? std::pair(first == p, NJ)
            :  p[1] == C('W')? std::pair(true, first == p? WJ : -WJ)
            :  /* else */      std::pair(false, NJ);
    }

    J::rep r    = 0;
    J::rep prev = 0;
    for (; p != q && prev <= r; ++p) {
        if (!isdigit(C::rep(*p))) return std::pair(false, NJ);
        prev = r;
        r = r * 10 + C::rep(*p) - '0';
    }
    return r < prev? std::pair(false, NJ)
        :  /*else*/  std::pair(true, J(*first != C('-')? r : -r));
}

std::pair<bool, I> parse_int::operator()(const C* first, const C* last) const {
    if (first == last) return std::pair(false, NI);

    const C*       p = first + (*first      == C('-'));
    const C* const q = last  - (*(last - 1) == C('i'));
    if (*p == C('0') && p + 2 == q && !isdigit(C::rep(p[1]))) {
        return p[1] == C('N')? std::pair(first == p, NI)
            :  p[1] == C('W')? std::pair(true, first == p? WI : -WI)
            :  /* else */      std::pair(false, NI);
    }

    I::rep r    = 0;
    I::rep prev = 0;
    for (; p != q && prev <= r; ++p) {
        if (!isdigit(C::rep(*p))) return std::pair(false, NI);
        prev = r;
        r = r * 10 + C::rep(*p) - '0';
    }
    return r < prev? std::pair(false, NI)
        :  /*else*/  std::pair(true, I(*first != C('-')? r : -r));
}

std::pair<bool, S> parse_sym::operator()(const C* first, const C* last) const {
    return std::pair(true, sym(first, last));
}

std::pair<bool, T> parse_time::operator()(const C* first, const C* last) const {
    if (first == last) return std::pair(false, NT);

    const C* const p = first + (*first      == C('-'));
    const C* const q = last  - (*(last - 1) == C('t'));
    if (*p == C('0') && p + 2 == q && !isdigit(C::rep(p[1]))) {
        return p[1] == C('N')? std::pair(first == p, NT)
            :  p[1] == C('W')? std::pair(true, first == p? WT : -WT)
            :  /* else */      std::pair(false, NT);
    }

    if (p + 12 != q) return std::pair(false, NT);
    const index_t di[] = {0, 1, 3, 4, 6, 7, 9, 10, 11};
    for (size_t i = 0; i < sizeof di / sizeof *di; ++i)
        if (!isdigit(C::rep(p[di[i]])))
            return std::pair(false, NT);
    const int h  = from_digits(p + 0, p + 2);
    const int m  = from_digits(p + 3, p + 5);
    const int s  = from_digits(p + 6, p + 8);
    const int ms = from_digits(p + 9, p + 12);
    if (m >= 60 || s >= 60) return std::pair(false, NT);
    const T r = hmsm2time(h, m, s, ms);
    return std::pair(true, first == p? r : -r);
}
