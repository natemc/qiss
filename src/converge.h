#pragma once

#include <l.h>
#include <match.h>
#include <o.h>
#include <object.h>
#include <type_traits>
#include <utility>

const struct Converge {
    template <class F, class X> requires std::is_invocable_r_v<X, F, X>
    X operator()(F f, X x) const {
        X prev(x);
        X result = f(prev);
        while (!match_(result, prev) && !match_(result, x)) {
            prev = result;
            result = f(std::move(result));
        }
        return result;
    }

    template <class F, class X> requires (
           std::is_invocable_v<F, X>
        && !std::is_same_v<std::invoke_result_t<F, X>, X>
        && std::is_invocable_r_v<std::invoke_result_t<F, X>, F, std::invoke_result_t<F, X>>)
    auto operator()(F f, X x) const {
        auto prev(x);
        auto result = f(prev);
        while (!match_(result, prev)) {
            prev = result;
            result = f(std::move(result));
        }
        return result;
    }
} converge;

const struct Convergence {
    template <class F, class X> requires std::is_invocable_r_v<X, F, X>
    L<X> operator()(F f, X x) const {
        L<X> r{x, f(x)};
        while (!match_(r.back(), r[r.size() - 1]) && !match_(r.back(), x))
            r.push_back(f(r.back()));
        return r;
    }

    template <class F, class X> requires (
           std::is_invocable_v<F, X>
        && !std::is_same_v<std::invoke_result_t<F, X>, X>
        && std::is_invocable_r_v<std::invoke_result_t<F, X>, F, std::invoke_result_t<F, X>>)
    auto operator()(F f, X x) const {
        using R = std::decay_t<decltype(f(x))>;
        L<R> r{R(x), f(x)};
        while (!match_(r.back(), r[r.size() - 1])) r.push_back(f(r.back()));
        return r;
    }
} convergence;

O converge1(ufun_t f, O x);
