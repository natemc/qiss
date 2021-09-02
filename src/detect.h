#pragma once

#include <type_traits>

// See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4502.pdf

// If any template param in ... fails to compile, so does void_t<...>
template <class...> using void_t = void;

// detect<T, Prop>::value is false
template <class, template <class> class, class=void_t<>>
struct detect: std::false_type {};

// unless Prop<T> is a valid type
template <class T, template <class> class Prop>
struct detect<T, Prop, void_t<Prop<T>>>: std::true_type {};
