#pragma once

#include <l.h>
#include <o.h>

O key(O x);

const struct TilI {
    L<I> operator()(index_t n) const;
    L<I> operator()(I       n) const { return (*this)(I::rep(n)); }
    L<I> operator()(J       n) const { return (*this)(J::rep(n)); }
    L<I> operator()(O       n) const;
} tili;

const struct Til {
    L<J> operator()(index_t n) const;
    L<J> operator()(I       n) const { return (*this)(I::rep(n)); }
    L<J> operator()(J       n) const { return (*this)(J::rep(n)); }
    L<J> operator()(O       n) const;
} til;
