#pragma once

#include <bits.h>
#include <cassert>
#include <cstdint>
#include <hash.h>
#include <match.h>
#include <l.h>
#include <o.h>
#include <prim.h>
#include <primio.h>

template <class Z> struct HashSet {
    static constexpr uint32_t MAX_DIST = 32;

    explicit HashSet(L<Z> x);

    bool    insert(index_t i);

    index_t find  (Z x) const;
    bool    has   (Z x) const { return find(x) != size(); }
    index_t size  ()    const { return count; }

private:
    L<Z>     items;
    index_t  capacity;
    index_t  count;
    L<J>     indices;
    L<X>     poverty; // distance from ideal slot in indices
    uint32_t logcap;

    index_t hash              (Z x) const;
    bool    insert_not_present(index_t h, J i);
    void    rehash            ();
    void    steal             (index_t h, uint8_t d, index_t i);

    template <class X> static void swap(X& x, typename X::rep& y) {
        const typename X::rep tmp = y;
        y = typename X::rep(x);
        x = X(tmp);
    }
};

template <class Z> HashSet<Z>::HashSet(L<Z> x):
    items(std::move(x)),
    capacity(32),
    count(0),
    indices(capacity),
    poverty(capacity),
    logcap(uint32_t(log2u64(uint64_t(capacity))))
{
    std::fill(indices.begin(), indices.end(), J(-1));
    std::fill(poverty.begin(), poverty.end(), X(0));
}

template <class Z> index_t HashSet<Z>::find(Z x) const {
    index_t h = hash(x); // slot
    uint8_t d = 0;       // distance from ideal slot
    if (indices.size() < h + MAX_DIST) { // risk of falling off end
        for (; h<indices.size() && indices[h]!=J(-1) && X(d)<=poverty[h]; ++h, ++d) {
            const index_t i(indices[h]);
            if (match_(items[i], x)) return i;
        }
        if (h != indices.size()) return size();
        h = 0;
    }
    for (; indices[h] != J(-1) && X(d) <= poverty[h]; ++h, ++d) {
        const index_t i(indices[h]);
        if (match_(items[i], x)) return i;
    }
    return size();
}

template <class Z>
index_t HashSet<Z>::hash(Z x) const {
    const index_t h = Hash()(logcap, x);
    assert(0ll <= h && h < indices.size());
    return index_t(h);
}

template <class Z> void HashSet<Z>::steal(index_t h, uint8_t d, index_t i) {
    if (poverty[h] < X(d)) {
        swap(indices[h], i);
        swap(poverty[h], d);
    }
}

template <class Z> bool HashSet<Z>::insert(index_t i) {
    assert(0 <= i && i < items.size());

    if (capacity / 10 * 9 <= size()) {
//        H(1) << " !! capacity rehash\n" << flush;
        rehash();
    }
    const Z x = items[i];
    index_t h = hash(x);
    uint8_t d = 0; // distance from ideal slot
    while (indices[h] != J(-1)) {
        for (; h < indices.size() && indices[h] != J(-1) && d < MAX_DIST; ++h, ++d) {
            if (match_(items[J::rep(indices[h])], x)) return false;
            steal(h, d, i);
        }
        if (h == indices.size()) {
            for (h = 0; indices[h] != J(-1) && d < MAX_DIST; ++h, ++d) {
                if (match_(items[J::rep(indices[h])], x)) return false;
                steal(h, d, i);
            }
        }
        if (d == MAX_DIST) {
//            H(1) << " !! poverty rehash\n" << flush;
            rehash();
            h = hash(x);
            d = 0;
        }
    }
    indices[h] = J(i);
    poverty[h] = X(d);
    ++count;
    return true;
}

template <class Z> bool HashSet<Z>::insert_not_present(index_t h, J i) {
    // When we know an item isn't present, we don't need to do an equals test.
    uint8_t d = 0; // distance from ideal slot
    while (indices[h] != J(-1) && d < MAX_DIST) {
        for (; h < indices.size() && indices[h] != J(-1) && d < MAX_DIST; ++h, ++d)
            steal(h, d, J::rep(i));
        if (h == indices.size()) {
            for (h = 0; indices[h] != J(-1) && d < MAX_DIST; ++h, ++d)
                steal(h, d, J::rep(i));
        }
    }

    // I've observed this during a rehash so we need to handle it.
    if (d == MAX_DIST) return false;

    indices[h] = i;
    poverty[h] = X(d);
    return true;
}

template <class Z> void HashSet<Z>::rehash() {
    capacity *= 2;
    L<J> old_indices(capacity);
    L<X> old_poverty(capacity);
    std::fill(old_indices.begin(), old_indices.end(), J(-1));
    std::fill(old_poverty.begin(), old_poverty.end(), X(0));
    logcap = uint32_t(log2u64(uint64_t(capacity)));
    assert(logcap < 32);
    std::swap(old_indices, indices);
    std::swap(old_poverty, poverty);
    for (index_t i = 0; i < old_indices.size(); ++i) {
        if (old_indices[i] != J(-1)) {
            const Z x = items[old_indices[i]];
            if (!insert_not_present(hash(x), old_indices[i])) {
                std::swap(old_indices, indices);
                std::swap(old_poverty, poverty);
                rehash();
                return;
            }
        }
    }
}
