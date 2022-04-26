#include <object.h>
#include <algorithm>
#include <count.h>
#include <fixed_size_allocator.h>
#include <o.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class F, class X> bool all(F&& f, const List<X>* x) {
        return std::all_of(x->begin(), x->end(), std::forward<F>(f));
    }

    bool is_rectangular(const List<O>* x) {
        return all([=](O e){ return e.get()->n == (*x)[0].get()->n; }, x);
    }

    using Alloc = FixedSizeAllocator<Object>;
    typename std::aligned_storage<sizeof(Alloc), alignof(Alloc)>::type buf;
    Alloc& oalloc = reinterpret_cast<Alloc&>(buf);
    uint64_t init_counter;

    Object* alloc_object(Type type) {
        Object* const o = oalloc.alloc();
        o->a     = Attr::none;
        o->apadv = Opcode(0);
        o->arity = 0;
        o->r     = 0;
        o->type  = type;
        return o;
    }
}

namespace qiss_object_detail {
    QissObjectInit:: QissObjectInit() { if (!init_counter++) new (&oalloc) Alloc; }
    QissObjectInit::~QissObjectInit() { if (!--init_counter) oalloc.~Alloc(); }
}

Object* alloc_atom(Type t) {
    assert(int(t) < 0);
    return alloc_object(t);
}

Object* generic_null() {
    static Object gn = {1, Opcode(0), 0, Attr::none, generic_null_type};
    return addref(&gn);
}

Dict* make_dict(Object* k, Object* v) {
    assert(count(O(addref(k))).atom<J>() == count(O(addref(v))).atom<J>());
    const auto [p, sz] = qiss_alloc(sizeof(Dict));
    Dict* const d = new (p) Dict;
    d->a     = Attr::none;
    d->apadv = Opcode(0);
    d->arity = 0;
    d->n     = 0;
    d->r     = 0;
    d->type = Type('!');
    d->k = k;
    d->v = v;
    return d;
}

Object* make_table(Dict* x) {
    assert(dk(x)->type == OT<S>::typet());
    assert(dv(x)->type == OT<O>::typet());
    assert(dk(x)->n == dv(x)->n);
    assert(is_rectangular(OT<O>::list(dv(x))));
    Object* const r = alloc_object(Type('+'));
    r->dict = x;
    return r;
}

index_t table_size(const Object* x) {
    assert(is_table(x));
    List<Object*>* const v = static_cast<List<Object*>*>(dv(x->dict));
    assert(v->n);
    return (*v)[0]->n;
}

Object* make_proc(S module, iaddr_t entry, X arity) {
    Object* const r = alloc_atom(-OT<Proc>::typet());
    r->arity        = X::rep(arity);
    r->proc.entry   = entry;
    r->proc.module  = module;
    return r;
}

Dict* make_keyed_table(Object* x, Object* y) {
    assert(is_table(x) && is_table(y) && table_size(x) == table_size(y));
    return make_dict(x, y);
}

void destruct_list(Object* o) {
#define CS(X) case OT<X>::typei(): OT<X>::list(o)->~List<X>(); break
    switch (int(o->type)) {
    CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X); CS(O);
    default: assert(false);
    }
#undef CS
}

void Deref::free_object(Object* x) {
    assert(x->r == uint32_t(-1));
    assert(x->type != generic_null_type);
    if (is_atom(x))
        oalloc.free(x);
    else if (is_dict(x)) {
        Dict* const d = dict(x);
        deref(d->k);
        deref(d->v);
        d->~Dict();
        qiss_free(x);
    } else if (is_table(x)) {
        deref(x->dict);
        oalloc.free(x);
    } else if (is_list(x)) {
        destruct_list(x);
        qiss_free(x);
    }
    else {
        assert(false);
    }
}
