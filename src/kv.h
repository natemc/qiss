#pragma once

#include <algorithm>
#include <indexof.h>
#include <l.h>
#include <o.h>
#include <utility>

template <class K, class V> struct KV {
    using size_type = index_t;

    KV(): KV(make_list<K>(), make_list<V>()) {}
    explicit KV(O x_): KV(x_.release()) {}
    explicit KV(Object* x_): KV(dict(x_)) { assert(x_); }
    explicit KV(Dict* x_): d(x_) {
        assert(x_);
        if constexpr(is_prim_v<K>)
            assert(dk(d)->type == ObjectTraits<K>::typet());
        else
            assert(dk(d)->type == ObjectTraits<O>::typet());
        if constexpr(is_prim_v<V>)
            assert(dv(d)->type == ObjectTraits<V>::typet());
        else
            assert(dv(d)->type == ObjectTraits<O>::typet());
    }
    KV(List<K>* k, List<V>* v): KV(make_dict(k, v)) {}
    KV(L<K> k, L<V> v): KV(addref(k.get()), addref(v.get())) {}
    KV(const KV& x_): d(addref(x_.d)) {}
    KV(KV&& x_): d(x_.release()) {}
    ~KV() { deref(d); }
    KV& operator=(KV x_) { std::swap(d, x_.d); return *this; }

    operator O()       &  { return O(addref(d)); }
    operator O() const &  { return O(KV(*this).release()); }
    operator O()       && { return O(release()); }

    bool      has (K x) const { return index(x) != size(); }
    size_type size()    const { return dk(d)->n; }
    L<K>      key ()          { return L<K>(addref(dk(d))); }
    L<V>      val ()          { return L<V>(addref(dv(d))); }

    V at(K x, V def) const {
        const index_t i = index(x);
        return i < size()? (*static_cast<List<V>*>(dv(d)))[i] : def;
    }

    V& operator[](K x) {
        const index_t i = index(x);
        assert(i < size());
        return (*static_cast<List<V>*>(dv(d)))[i];
    }
    const V& operator[](K x) const { return const_cast<KV*>(this)[x]; }

    void add(K x, V y) {
        const index_t oldsz = size();
        List<K>* const k = grow_list(static_cast<List<K>*>(dk(d)), uint64_t(oldsz + 1));
        List<V>* const v = grow_list(static_cast<List<V>*>(dv(d)), uint64_t(oldsz + 1));
        new (k->end()) K(x);
        new (v->end()) V(y);
        ++k->n;
        ++v->n;
        get()->k = k;
        get()->v = v;
    }

    void remove(K x) {
        List<K>* const k = static_cast<List<K>*>(dk(d));
        List<V>* const v = static_cast<List<V>*>(dv(d));
        const index_t  i = index(x);
        if (i == size()) return;
        (*k)[i].~K();
        (*v)[i].~V();
        std::copy(k->begin() + i + 1, k->end(), k->begin() + i);
        std::copy(v->begin() + i + 1, v->end(), v->begin() + i);
        k->n = --v->n;
    }

    void upsert(K x, V y) {
        const index_t i = index(x);
        if (i == size()) add(std::move(x), std::move(y));
        else             (*static_cast<List<V>*>(dv(d)))[i] = y;
    }

    Dict* release() { return dict(std::exchange(d, generic_null())); }

private:
    index_t index(K x) const { return indexof(L<K>(addref(dk(d))), x); }
    Object* d;
    Dict* get() { return dict(d); }
};
