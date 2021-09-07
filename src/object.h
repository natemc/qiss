#pragma once

#include <bits.h>
#include <iaddr.h>
#include <cstdint>
#include <index.h>
#include <memory>
#include <opcode.h>
#include <prim.h>
#include <qiss_alloc.h>
#include <type_traits>
#include <utility>

// This guarantees that the allocator outlives all users.
// See https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter
namespace qiss_object_detail {
    static const struct QissObjectInit { QissObjectInit(); ~QissObjectInit(); } init;
}

enum class Attr: uint8_t { none, grouped, parted, sorted = 4, unique = 8 };
inline Attr operator&(Attr x, Attr y) {
    return Attr(std::underlying_type_t<Attr>(x) & std::underlying_type_t<Attr>(y));
}
inline Attr operator|(Attr x, Attr y) {
    return Attr(std::underlying_type_t<Attr>(x) | std::underlying_type_t<Attr>(y));
}

struct Type {
    Type() = default;
    explicit constexpr Type(int8_t t_): t(t_) {}
    explicit constexpr operator int() const { return t; }
    constexpr Type operator-() const { return Type(-t); }
private:
    int8_t t;
};
constexpr inline bool operator==(Type x, Type y) { return int(x) == int(y); }
constexpr inline bool operator!=(Type x, Type y) { return !(x == y); }
constexpr inline bool operator< (Type x, Type y) { return int(x) < int(y); }
constexpr inline bool operator<=(Type x, Type y) { return !(y < x); }
constexpr inline bool operator> (Type x, Type y) { return y < x; }
constexpr inline bool operator>=(Type x, Type y) { return !(x < y); }
constexpr inline bool operator==(Type x, int  y) { return int(x) == y; }
constexpr inline bool operator!=(Type x, int  y) { return !(x == y); }
constexpr inline bool operator< (Type x, int  y) { return int(x) < y; }
constexpr inline bool operator<=(Type x, int  y) { return !(y < int(x)); }
constexpr inline bool operator> (Type x, int  y) { return y < int(x); }
constexpr inline bool operator>=(Type x, int  y) { return !(x < y); }
constexpr inline bool operator==(int  x, Type y) { return x == int(y); }
constexpr inline bool operator!=(int  x, Type y) { return !(x == y); }
constexpr inline bool operator< (int  x, Type y) { return x < int(y); }
constexpr inline bool operator<=(int  x, Type y) { return !(y < x); }
constexpr inline bool operator> (int  x, Type y) { return y < x; }
constexpr inline bool operator>=(int  x, Type y) { return !(x < y); }

struct Dict;
struct O;
using nfun_t = O(*)();
using ufun_t = O(*)(O);
using bfun_t = O(*)(O,O);
using tfun_t = O(*)(O,O,O);
struct Proc { S module; iaddr_t entry; };
template <> struct is_prim<nfun_t>: std::true_type {};
template <> struct is_prim<ufun_t>: std::true_type {};
template <> struct is_prim<bfun_t>: std::true_type {};
template <> struct is_prim<tfun_t>: std::true_type {};
template <> struct is_prim<Opcode>: std::true_type {};
constexpr Type generic_null_type{-128};
constexpr Type dict_type        {'!'};
constexpr Type table_type       {'+'};

struct AO { Opcode op, adverb; };
template <> struct is_prim<AO>: std::true_type {};
inline bool operator==(AO x, AO y) { return x.op == y.op && x.adverb == y.adverb; }

struct Object {
    uint32_t r;
    uint16_t m; // reserved
    Attr     a;
    Type     type;
    union {
        B       b;
        C       c;
        D       d;
        Dict*   dict;
        F       f;
        H       h;
        I       i;
        J       j;
        S       s;
        T       t;
        X       x;
        index_t n;
        AO      ao;
        Opcode  op;
        nfun_t  nfun;
        ufun_t  ufun;
        bfun_t  bfun;
        tfun_t  tfun;
        Proc    proc;
    };
};
static_assert(sizeof(Object) == 16);

template <class X> struct IsObject:
    std::bool_constant<std::is_base_of_v<Object, std::remove_pointer_t<X>>>
{};
template <class X> inline constexpr bool is_object_v = IsObject<X>::value;

[[nodiscard]] Object* alloc_atom(Type t);
inline bool is_atom(const Object* x) { return x->type < 0; }
inline bool is_primitive_atom(const Object* x) { return is_atom(x) && Type(-20) < x->type; }
[[nodiscard]] Object* generic_null();

const struct Addref {
    template <class X> X* operator()(X* x) const { ++x->r; return x; }
} addref;

const struct Deref {
    void operator()(Object* o) const { if (!o->r--) free_object(o); }
private:
    static void free_object(Object* o);
} deref;

template <class X> struct List: Object {
    List(): hash_cache(uint64_t(-1)) {
        static_assert(sizeof(List) == 24);
        if constexpr(is_prim_v<X>)
            static_assert(sizeof(X[2]) == sizeof(typename X::rep[2]));
        assert(((alignof(X) - 1) & std::size_t(bitcast<std::uintptr_t>(this + 1))) == 0);
        assert((bitcast<std::uintptr_t>(const_cast<const List*>(this)->begin()) & 31) == 0);
        //assert((bitcast<std::uintptr_t>(std::as_const(this)->begin()) & 31) == 0);
    }
    ~List() { std::destroy(begin(), end()); }

    X&       operator[](index_t i)       { return begin()[i]; }
    const X& operator[](index_t i) const { return (*const_cast<List*>(this))[i]; }
    X&       operator[](J       i)       { return (*this)[J::rep(i)]; }
    const X& operator[](J       i) const { return (*this)[J::rep(i)]; }

    X*       begin     ()                {
        return reinterpret_cast<X*>(first);
        // clang crashes :-(
//        return static_cast<X*>(__builtin_assume_aligned(first, 32));
    }
    const X* begin     ()          const { return const_cast<List*>(this)->begin(); }
    X*       end       ()                { return begin() + n; }
    const X* end       ()          const { return const_cast<List*>(this)->end(); }

private:
    uint64_t hash_cache; // just an idea (we do need the bytes to align first)
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    char     first[];
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
};

struct Dict: Object {
    Object* k;
    Object* v;
};
[[nodiscard]] Dict* make_dict(Object* k, Object* v);
inline bool          is_dict(const Object* x) { return  x->type == '!'; }
inline Dict*         dict   (Object*       x) { assert(is_dict(x)); return static_cast<Dict*>(x); }
inline const Dict*   dict   (const Object* x) { return dict(const_cast<Object*>(x)); }
inline Object*       dk     (Object*       x) { return dict(x)->k; }
inline const Object* dk     (const Object* x) { return dk(const_cast<Object*>(x)); }
inline Object*       dv     (Object*       x) { return dict(x)->v; }
inline const Object* dv     (const Object* x) { return dv(const_cast<Object*>(x)); }
inline index_t       dict_size(const Dict* x) { return dk(x)->n; }
[[nodiscard]] inline std::pair<Object*, Object*> unwrap_dict(Dict* x) {
    assert(!x->r);
    Object* const k = x->k;
    Object* const v = x->v;
    x->~Dict();
    qiss_free(x);
    return std::pair(k, v);
}

template <class X> struct ObjectTraits {
    static constexpr int  typei() { return 0; }
    static constexpr Type typet() { return Type(0); }
};
#define OTRAITS(N,X,C,T,M,U) template <> struct ObjectTraits<X> {\
    using type = X;                                              \
    static constexpr char     ch   () { return C; }              \
    static constexpr X&       get  (Object* o) {                 \
        assert(o->type == -T); return o->M; }                    \
    static constexpr X        get  (const Object* o) {           \
        assert(o->type == -T); return o->M; }                    \
    static constexpr List<X>* list (Object* o) {                 \
        assert(o->type == T); return static_cast<List<X>*>(o); } \
    static constexpr const List<X>* list (const Object* o) {     \
        return list(const_cast<Object*>(o)); }                   \
    static constexpr const char*    name () { return #N; }       \
    static constexpr  X null () { return U; }                    \
    static constexpr Object*  set  (Object* o, X x) {            \
        assert(o->type == -T); o->M = x; return o; }             \
    static constexpr int      typei() { return T; }              \
    static constexpr Type     typet() { return Type(T); }        \
}
OTRAITS(bool  , B     , 'b',  1, b   , B(0));
OTRAITS(char  , C     , 'c',  2, c   , C(' '));
OTRAITS(date  , D     , 'd',  3, d   , ND);
OTRAITS(float , F     , 'f',  5, f   , NF);
OTRAITS(handle, H     , 'h',  6, h   , NH);
OTRAITS(int   , I     , 'i',  7, i   , NI);
OTRAITS(long  , J     , 'j',  8, j   , NJ);
OTRAITS(sym   , S     , 's',  9, s   , NS);
OTRAITS(time  , T     , 't', 10, t   , NT);
OTRAITS(byte  , X     , 'x', 11, x   , X(0));
OTRAITS(advop , AO    , '/', 12, ao  , AO({Opcode::dot, Opcode::over}));
OTRAITS(op    , Opcode, '.', 13, op  , Opcode::dot);
OTRAITS(nfun  , nfun_t, '(', 14, nfun, nullptr);
OTRAITS(ufun  , ufun_t, '(', 15, ufun, nullptr);
OTRAITS(bfun  , bfun_t, '(', 16, bfun, nullptr);
OTRAITS(tfun  , tfun_t, '(', 17, tfun, nullptr);
OTRAITS(proc  , Proc  , '(', 19, proc, Proc());

const struct MakeAtom {
    template <class X> [[nodiscard]] Object* operator()(X x) const {
        return ObjectTraits<X>::set(alloc_atom(-ObjectTraits<X>::typet()), x);
    }
} make_atom;

template <class X> [[nodiscard]] List<X>* make_empty_list(index_t cap = 0) {
    const auto [p, sz] = qiss_alloc(sizeof(List<X>) + std::size_t(cap) * sizeof(X));
    List<X>* const lst = new (p) List<X>;
    lst->a    = Attr::none;
    lst->m    = 0;
    lst->r    = 0;
    lst->n    = 0;
    lst->type = ObjectTraits<X>::typet();
    return lst;
}

inline bool is_list(Object* x) { return 0 <= x->type && x->type < 20; }

template <class X> [[nodiscard]] List<X>* make_list(index_t n = 0) {
    List<X>* const lst = make_empty_list<X>(n);
    std::uninitialized_default_construct_n(lst->begin(), n);
    lst->n = n;
    return lst;
}

template <class X> uint64_t list_capacity(const List<X>* x) {
    return (qiss_alloc_size(x) - sizeof(List<X>)) / sizeof(X);
}

template <class X>
[[nodiscard]] List<X>* grow_list(List<X>* orig, uint64_t new_n) {
    auto [g, newsz] = qiss_grow(orig, sizeof(List<X>) + new_n * sizeof(X));
    if (g) { assert(g == orig); return orig; }

    const auto [p, sz] = qiss_alloc(sizeof(List<X>) + new_n * sizeof(X));
    List<X>* const lst = new (p) List<X>;
    std::uninitialized_copy_n(orig->begin(), orig->n, lst->begin());
    lst->a    = Attr::none;
    lst->m    = 0;
    lst->r    = 0;
    lst->n    = orig->n;
    lst->type = orig->type;
    deref(orig);
    return lst;
}

[[nodiscard]] Object* make_table(Dict* x);
inline Object* make_table(Object* cols, Object* cells) {
    return make_table(make_dict(cols, cells));
}
inline bool is_table(const Object* x) { return x->type == '+'; }
index_t table_size(const Object* x);

[[nodiscard]] Dict* make_keyed_table(Object* x, Object* y);
inline bool is_keyed_table(const Object* x) {
    return is_dict(x) && is_table(dk(x)) && is_table(dv(x));
}

[[nodiscard]] Object* make_proc(S module, iaddr_t entry, X arity);
X proc_arity(Object* x);

const struct Box {
    template <class X> requires is_prim_v<X> Object operator()(X x) const {
        Object r{1, 0, Attr::none, -ObjectTraits<X>::typet()};
        return *ObjectTraits<X>::set(&r, x);
    }
} box;
