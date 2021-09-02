#pragma once

// Advantage over hash table: an in-order traversal (O(n))
// yields the lexicographical order of all symbols.
// TODO try compressing tails, AMT

#include <cmath>
#include <cstdint>
#include <fixed_size_allocator.h>
#include <l.h>
#include <limits>
#include <utility>

struct SymTrie {
    SymTrie();
    ~SymTrie();

    const char* c_str(S x) const;
    std::pair<bool, S> insert(const char* first, const char* last);
    std::pair<bool, S> insert(const char* s);
    std::size_t        memory() const;
    const L<J>&        rank  () const;
    int32_t            size  () const {
        assert(positions.size() <= std::numeric_limits<int32_t>::max());
        return int32_t(positions.size());
    }

    friend H operator<<(H h, const SymTrie& t);

private:
    static const S::rep EMPTY = std::numeric_limits<S::rep>::max();
    struct Node {
        Node();
        ~Node();
        Node* children[16];
        S::rep symbol; // EMPTY or an index into positions.
    };
    FixedSizeAllocator<SymTrie::Node> nalloc;

    L<C> symbols;   // symbols, null-terminated, end-to-end
    L<I> positions; // index into symbols
    Node root;

    enum hilo { HIGH, LOW };

    void               free  (Node* n);
    std::pair<bool, S> find  (const Node* n, const char* s, hilo hl) const;
    std::pair<bool, S> find  (const Node* n, const char* first, const char* last,
                              hilo hl) const;
    std::pair<bool, S> insert(Node* n, const char* first, const char* last,
                              const char* i, hilo hl);

    template <class F> static void preorder(const SymTrie::Node* n, F f) {
        if (n->symbol != SymTrie::EMPTY) f(std::abs(n->symbol));
        if (n->symbol == SymTrie::EMPTY || 0 <= n->symbol) {
            for (auto c: n->children) if (c) preorder(c, f);
        }
    }

    H print(H h, const SymTrie::Node* n, L<X>& s, int indent=0) const;
};
