#pragma once

#include <object.h>
#include <utility>

struct O {
    template <class X> using OT = ObjectTraits<X>;

    O(): x(generic_null()) {}
    template <class X> requires is_prim_v<X> explicit O(X x_): x(make_atom(x_)) {
    }
    explicit O(Object* x_): x(x_) { assert(x_); }
    O(const O& x_): x(addref(x_.x)) {}
    O(O&& x_): x(x_.release()) {}
    ~O() { deref(x); }
    O& operator=(O x_) { std::swap(x, x_.x); return *this; }

    bool                  is_atom      () const { return ::is_atom (x); }
    bool                  is_dict      () const { return ::is_dict (x); }
    bool                  is_list      () const { return ::is_list (x); }
    bool                  is_table     () const { return ::is_table(x); }

    Type                  type         () const { assert(x); return x->type; }
    template <class X> X& atom         ()       { assert(x); return OT<X>::get(x); }
    template <class X> X  atom         () const { assert(x); return OT<X>::get(x); }

    Object*               operator->   ()       { return x; }
    const Object*         operator->   () const { return x; }
    Object*               get          ()       { return x; }
    const Object*         get          () const { return x; }
    Object*               release      ()       { return std::exchange(x, generic_null()); }

private:
    Object* x;
};

template <> struct ObjectTraits<O> {
    using type = O;
    static constexpr List<O>*       list (Object* o)       {
        assert(o->type == 0); return static_cast<List<O>*>(o);
    }
    static constexpr const List<O>* list (const Object* o) {
        return list(const_cast<Object*>(o));
    }
    static constexpr const char*    name () { return "any"; }
    static O                        null () { return O(generic_null()); }
    static constexpr int            typei() { return 0; }
    static constexpr Type           typet() { return Type(0); }
};
