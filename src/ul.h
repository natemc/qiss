#pragma once

#include <o.h>
#include <utility>

struct [[nodiscard]] UL { // untyped list
    using size_type = index_t;

    explicit UL(Object* x_): x(x_) { assert(x_); assert(is_list(x_)); }
    explicit UL(O x_): UL(x_.release()) {}
    UL(const UL& x_): x(addref(x_.x)) {}
    UL(UL&& x_): x(x_.release()) {}
    ~UL() { deref(x); }
    UL operator=(UL x_) { std::swap(x, x_.x); return *this; }

    operator O()       &  { return O(addref(x)); }
    operator O() const &  { return O(UL(*this).release()); }
    operator O()       && { return O(release()); }

    size_type size() const { return x->n; }

    Object*       operator->()       { return x; }
    const Object* operator->() const { return x; }
    Object*       get       ()       { return x; }
    const Object* get       () const { return x; }
    Object*       release   ()       { return std::exchange(x, generic_null()); }

private:
    Object* x;
};
