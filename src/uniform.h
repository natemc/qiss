#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <l.h>
#include <o.h>
#include <type_traits>
#include <utility>

template <class IT>
    requires std::is_same_v<O, std::decay_t<decltype(*std::declval<IT>())>>
bool are_uniform_atoms(IT first, IT last) {
    if (first == last) return false;
    const auto same_type = [=](const O& x){ return x.type() == first->type(); };
    return first->is_atom() && std::all_of(first, last, same_type);
}

// IT is often O*, but it might be, e.g., reverse_iterator<O>
template <class X, class IT> 
    requires std::is_same_v<O, std::decay_t<decltype(*std::declval<IT>())>>
L<X> uniform(IT first, IT last) {
    assert(are_uniform_atoms(first, last));
    L<X> r(std::distance(first, last));
    std::transform(first, last, r.begin(), [](const O& x){ return x.atom<X>(); });
    return r;
}

template <class IT> 
    requires std::is_same_v<O, std::decay_t<decltype(*std::declval<IT>())>>
O uniform(IT first, IT last) {
    assert(first != last);
#define CS(X) case -ObjectTraits<X>::typei(): return uniform<X>(first, last);
    switch (int((*first).type())) {
    CS(B); CS(C); CS(D); CS(F); CS(I); CS(J); CS(S); CS(T); CS(X);
    default: assert(false);
    }
    return O{}; // not reached
#undef CS
}
