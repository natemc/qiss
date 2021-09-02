#pragma once

#include <o.h>
#include <utility>

// Untyped Key-Value: smart pointer to a dict whose k & v types we don't know
struct [[nodiscard]] UKV {
    using size_type = index_t;

    explicit UKV(Object* x_): x(x_) { assert(x_); assert(is_dict(x_)); }
    explicit UKV(O x_): UKV(x_.release()) {}
    explicit UKV(Dict* x_): x(x_) { assert(x_); }
    UKV(O k, O v): x(make_dict(k.release(), v.release())) {}
    UKV(const UKV& x_): x(addref(x_.x)) {}
    UKV(UKV&& x_) noexcept: x(x_.release()) {}
    // clang-tidy says I'm deref'ing an uninitialized value (x) after move,
    // but x is always initialized, even after move
    ~UKV() { deref(x); } // NOLINT
    UKV& operator=(UKV x_) { std::swap(x, x_.x); return *this; }

    operator O()       &  { return O(addref(x)); }
    operator O() const &  { return O(UKV(*this).release()); }
    operator O()       && { return O(release()); }

    bool      mine() const { return !x->r; }
    size_type size() const { return dk(x)->n; }

    O key() &  { return O(addref(dk(x))); }
    O key() && {
        if (!mine()) return key();
        auto [k, v] = unwrap_dict(release());
        deref(v);
        return O(k);
    }
    O val() &  { return O(addref(dv(x))); }
    O val() && {
        if (!mine()) return val();
        auto [k, v] = unwrap_dict(release());
        deref(k);
        return O(v);
    }
    std::pair<O, O> kv() && {
        if (!mine()) return std::pair(key(), val());
        auto [k, v] = unwrap_dict(release());
        return std::pair(O(k), O(v));
    }

    Dict*       operator->()       { return dict(x); }
    const Dict* operator->() const { return dict(x); }
    Dict*       get       ()       { return dict(x); }
    const Dict* get       () const { return dict(x); }
    Dict*       release   ()       { return dict(std::exchange(x, generic_null())); }

private:
    Object* x;
};
