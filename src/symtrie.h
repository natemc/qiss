#pragma once

// Advantage over hash table: an in-order traversal (O(n))
// yields the lexicographical order of all symbols.
// TODO try compressing tails, AMT

#include <cmath>
#include <cstdint>
#include <l.h>
#include <limits>
#include <utility>

struct SymTrie {
    SymTrie();

    const char* c_str(S x) const {
        return reinterpret_cast<const char*>(&symbols[positions[S::rep(x)]]);
    }
    std::pair<bool, S> insert(const char* first, const char* last);
    std::pair<bool, S> insert(const char* s);
    std::size_t        memory() const;
    const L<J>&        rank  () const;
    int32_t            size  () const {
        assert(positions.size() <= std::numeric_limits<int32_t>::max());
        return int32_t(positions.size());
    }

    friend H operator<<(H h, const SymTrie& t);

    static const S::rep EMPTY = std::numeric_limits<S::rep>::max();
    struct Node {
        Node();
        ~Node();
        Node* children[16];
        S::rep symbol; // EMPTY or an index into positions.
    };

private:
    L<C> symbols;   // symbols, null-terminated, end-to-end
    L<I> positions; // index into symbols
    Node root;
    enum hilo { HIGH, LOW };
    std::pair<bool, S> find  (const Node* n, const char* s, hilo hl) const;
    std::pair<bool, S> insert(Node* n, const char* first, const char* last,
                              const char* i, hilo hl);
};
