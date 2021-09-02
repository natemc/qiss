#pragma once

#include <match.h>
#include <type_traits>
#include <utility>

const struct Converge {
    template <class F, class X> requires std::is_invocable_r_v<X, F, X>
    auto operator()(F f, X x) const {
        X prev(f(x));
        X result = f(prev);
        while (!match_(result, prev) && !match_(result, x)) {
            prev = result;
            result = f(std::move(result));
        }
        return result;
    }

    template <class F, class X> requires (std::is_invocable_v<F, X>
        && !std::is_same_v<std::invoke_result_t<F, X>, X>
        && std::is_invocable_r_v<std::invoke_result_t<F, X>, F, std::invoke_result_t<F, X>>)
    auto operator()(F f, X x) const {
        auto prev(f(x));
        auto result = f(prev);
        while (!match_(result, prev)) {
            prev = result;
            result = f(std::move(result));
        }
        return result;
    }
} converge_;

O converge(ufun_t f, O x);
