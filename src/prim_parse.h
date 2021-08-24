#pragma once

#include <l.h>
#include <utility>

bool all_digits(const C* first, const C* last);

#define PARSER(TYPE, NAME) const struct NAME {                             \
    std::pair<bool, TYPE> operator()(const C* first, const C* last) const; \
    std::pair<bool, TYPE> operator()(const L<C> x) const {                 \
        return (*this)(x.begin(), x.end());                                \
    }                                                                      \
} NAME

PARSER(B, parse_bool );
PARSER(C, parse_char );
PARSER(D, parse_date );
PARSER(F, parse_float);
PARSER(I, parse_int );
PARSER(J, parse_long );
PARSER(S, parse_sym  );
PARSER(T, parse_time );
PARSER(X, parse_byte );

#undef PARSER
