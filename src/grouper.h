#pragma once

// TODO handle Object*
// TODO Use the smallest possible value type
//      i.e., if x->n <= 256, use X for indices
// specialize?
// add EQ and HASH template parameters?

#include <algorithm>
#include <bits.h>
#include <cassert>
#include <cstdint>
#include <hash.h>
#include <l.h>
#include <match.h>
#include <o.h>
#include <primio.h>
#include <utility>

template <class Z> struct Grouper {
    static constexpr uint32_t MAX_DIST = 32;

    explicit Grouper(L<Z> x);
    Grouper(const Grouper&) = delete;
    Grouper& operator=(const Grouper&) = delete;

    L<Z>    key ()       { return key_; }
    L<L<J>> val ()       { return val_; }

    index_t size() const { return key_.size(); }

private:
    L<Z>     items;
    index_t  capacity;
    L<J>     indices;
    L<Z>     key_;
    L<L<J>>  val_;
    L<X>     poverty; // distance from ideal slot in indices
    uint32_t logcap;

    index_t hash              (Z x) const;
    void    insert            (index_t i);
    bool    insert_not_present(index_t h, uint8_t d, index_t i);
    void    rehash            ();

    template <class X> static void swap(X& x, typename X::rep& y) {
        const typename X::rep tmp = y;
        y = typename X::rep(x);
        x = X(tmp);
    }
};

template <class Z> Grouper<Z>::Grouper( L<Z> x):
    items(std::move(x)),
    capacity(32),
    indices(capacity),
    poverty(capacity),
    logcap(uint32_t(log2u64(uint64_t(capacity))))
{
    std::fill(indices.begin(), indices.end(), J(-1));
    std::fill(poverty.begin(), poverty.end(), X(0));
    for (index_t i = 0; i < items.size(); ++i) insert(i);
}

template <class Z> index_t Grouper<Z>::hash(Z x) const {
    const index_t h = Hash()(logcap, x);
    assert(0ll <= h && h < indices.size());
    return h;
}

template <class Z> void Grouper<Z>::insert(index_t i) {
    assert(0 <= i && i < items.size());

    if (capacity / 10 * 9 <= size()) {
        // H(1) << " !! capacity rehash\n" << flush;
        rehash();
    }

    const Z x  = items[i];
    index_t h  = hash(x);
    uint8_t d  = 0; // distance from ideal slot
    auto append_if_match = [&](){
        if (match_(key_[indices[h]], x)) {
            val_[indices[h]].emplace_back(i);
            return true;
        }
        if (poverty[h] < X(d)) {
            // By construction, if we reach here, x is new
            key_.emplace_back(x);
            const J::rep tmp(J::rep(std::exchange(indices[h], J(val_.size()))));
            val_.emplace_back(L<J>{J(i)});
            i = tmp;
            swap(poverty[h], d);
            insert_not_present(h, d, i);
            return true;
        }
        return false;
    };

    while (indices[h] != J(-1)) {
        for (; h < indices.size() && indices[h] != J(-1) && d < MAX_DIST; ++h, ++d) {
            if (append_if_match()) return;
        }
        if (h == indices.size()) {
            for (h = 0; indices[h] != J(-1) && d < MAX_DIST; ++h, ++d) {
                if (append_if_match()) return;
            }
        }
        if (d == MAX_DIST) {
            // H(1) << " !! poverty rehash\n" << flush;
            rehash();
            h = hash(x);
            d = 0;
        }
    }
    assert(h < indices.size());

    key_.emplace_back(x);
    indices[h] = J(val_.size());
    val_.emplace_back(L<J>{J(i)});
    poverty[h] = X(d);
}

template <class Z>
bool Grouper<Z>::insert_not_present(index_t h, uint8_t d, index_t i) {
    while (indices[h] != J(-1) && d < MAX_DIST) {
        for (; h < indices.size() && indices[h] != J(-1) && d < MAX_DIST; ++h, ++d) {
            if (poverty[h] < X(d)) {
                swap(indices[h], i);
                swap(poverty[h], d);
            }
        }
        if (h == indices.size()) {
            for (h = 0; indices[h] != J(-1) && d < MAX_DIST; ++h, ++d) {
                if (poverty[h] < X(d)) {
                    swap(indices[h], i);
                    swap(poverty[h], d);
                }
            }
        }
    }

    if (d == MAX_DIST) return false;

    indices[h] = J(i);
    poverty[h] = X(d);
    return true;
}

template <class Z> void Grouper<Z>::rehash() {
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
            const Z x = key_[old_indices[i]];
            if (!insert_not_present(hash(x), 0, J::rep(old_indices[i]))) {
                std::swap(old_indices, indices);
                std::swap(old_poverty, poverty);
                rehash();
                return;
            }
        }
    }
}
