#pragma once

#include <algorithm>
#include <indexof.h>
#include <l.h>
#include <o.h>
#include <utility>

template <class K, class V> struct [[nodiscard]] KV {
    using size_type = index_t;

    KV(): KV(make_list<K>(), make_list<V>()) {}
    explicit KV(O x_): KV(x_.release()) {}
    explicit KV(Object* x_): KV(dict(x_)) { assert(x_); }
    explicit KV(Dict* x_): x(x_) {
        assert(x_);
        if constexpr(is_prim_v<K>)
            assert(dk(x)->type == ObjectTraits<K>::typet());
        else
            assert(dk(x)->type == ObjectTraits<O>::typet());
        if constexpr(is_prim_v<V>)
            assert(dv(x)->type == ObjectTraits<V>::typet());
        else
            assert(dv(x)->type == ObjectTraits<O>::typet());
    }
    KV(List<K>* k, List<V>* v): KV(make_dict(k, v)) {}
    KV(L<K> k, L<V> v): KV(addref(k.get()), addref(v.get())) {}
    KV(const KV& x_): x(addref(x_.x)) {}
    KV(KV&& x_) noexcept: x(x_.release()) {}
    // clang-tidy says I'm deref'ing an uninitialized value (x) after move,
    // but x is always initialized, even after move
    ~KV() { deref(x); } // NOLINT
    KV& operator=(KV x_) { std::swap(x, x_.x); return *this; }

    operator O()       &  { return O(addref(x)); }
    operator O() const &  { return O(KV(*this).release()); }
    operator O()       && { return O(release()); }

    bool      has (K k) const { return index(k) != size(); }
    size_type size()    const { return dk(x)->n; }
    L<K>      key ()          { return L<K>(addref(dk(x))); }
    L<V>      val ()          { return L<V>(addref(dv(x))); }

    V at(K k, V def) const {
        const index_t i = index(k);
        return i < size()? (*static_cast<List<V>*>(dv(x)))[i] : def;
    }

    V& operator[](K k) {
        const index_t i = index(k);
        assert(i < size());
        return (*static_cast<List<V>*>(dv(x)))[i];
    }
    const V& operator[](K k) const { return const_cast<KV*>(this)[k]; }

    void add(K k, V v) {
        const index_t oldsz = size();
        List<K>* const k_ = grow_list(static_cast<List<K>*>(dk(x)), uint64_t(oldsz + 1));
        List<V>* const v_ = grow_list(static_cast<List<V>*>(dv(x)), uint64_t(oldsz + 1));
        new (k_->end()) K(k);
        new (v_->end()) V(v);
        ++k_->n;
        ++v_->n;
        get()->k = k_;
        get()->v = v_;
    }

    void remove(K k) {
        List<K>* const k_ = static_cast<List<K>*>(dk(x));
        List<V>* const v_ = static_cast<List<V>*>(dv(x));
        const index_t  i = index(k);
        if (i == size()) return;
        (*k_)[i].~K();
        (*v_)[i].~V();
        std::copy(k_->begin() + i + 1, k_->end(), k_->begin() + i);
        std::copy(v_->begin() + i + 1, v_->end(), v_->begin() + i);
        k_->n = --v_->n;
    }

    void upsert(K k, V v) {
        const index_t i = index(k);
        if (i == size()) add(std::move(k), std::move(v));
        else             (*static_cast<List<V>*>(dv(x)))[i] = v;
    }

    Dict* get    () { return dict(x); }
    Dict* release() { return dict(std::exchange(x, generic_null())); }

private:
    index_t index(K k) const { return indexof(L<K>(addref(dk(x))), k); }
    Object* x;
};
