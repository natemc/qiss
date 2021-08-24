#include <symtrie.h>

#include <algorithm>
#include <cassert>
#include <fixed_size_allocator.h>
//#include <iasc.h>
#include <limits>
#include <numeric>
//#include <primio.h>
#include <unistd.h>

namespace {
    L<J> iota(index_t n) {
        L<J> r(n);
        std::iota(r.begin(), r.end(), J(0));
        return r;
    }

    FixedSizeAllocator<SymTrie::Node>& nalloc() {
        static FixedSizeAllocator<SymTrie::Node> na;
        return na;
    }

    template <class F> static void preorder(const SymTrie::Node* n, F f) {
        if (n->symbol != SymTrie::EMPTY) f(std::abs(n->symbol));
        if (n->symbol == SymTrie::EMPTY || 0 <= n->symbol) {
            for (auto c: n->children) if (c) preorder(c, f);
        }
    }

    H print(H h, const SymTrie& t, const SymTrie::Node* n, L<X>& s, int indent=0) {
        //constexpr const char digit[] = "0123546789abcdef";
    
        for (int i = 0; i < indent  ; ++i) ;//h << ' ';
        for (int i = 0; i < s.size(); ++i) ;//h << digit[X::rep(s[i]) & 15];
        //if (n->symbol != EMPTY)
        //    h << " (" << n->symbol << "): `" << t.c_str(S(std::abs(n->symbol)));
        //h << '\n';

        for (std::size_t i = 0; i < std::size(n->children); ++i) {
            if (n->children[i]) {
                s.push_back(X(X::rep(i)));
                print(h, t, n->children[i], s, indent + 2);
                s.pop();
            }
        }
        return h;
    }
}

SymTrie::Node::Node(): symbol(EMPTY) {
    std::fill(std::begin(children), std::end(children), nullptr);
}

SymTrie::Node::~Node() {
    for (Node* c: children) { if (c) nalloc().free(c); }
}

SymTrie::SymTrie() {
    root.symbol = 0;
    positions.push_back(I(0));
    symbols.push_back(C('\0'));
}

std::pair<bool, S> SymTrie::insert(const char* s) {
    return insert(&root, s, s + strlen(s), s, HIGH);
}

std::pair<bool, S> SymTrie::insert(const char* first, const char* last) {
    if (symbols.size() + (last-first) >= std::numeric_limits<I::rep>::max()) {
        write(2, "'wsfull", 7);
        exit(1);
    }
    return first == last? std::pair(false, S(0))
        :                 insert(&root, first, last, first, HIGH);
}

std::pair<bool, S>
SymTrie::insert(Node* n, const char* first, const char* last, const char* i, hilo hl) {
    if (i == last) {
        if (n->symbol != EMPTY) return std::pair(false, S(n->symbol));
        n->symbol = S::rep(I::rep(positions.size()));
        positions.emplace_back(I::rep(symbols.size()));
        symbols.append(first, last);
        symbols.push_back(C('\0'));
        return std::pair(true, S(n->symbol));
    }

    if (hl == HIGH) {
        if (!n->children[*i >> 4]) n->children[*i >> 4] = nalloc().alloc();
        Node* const child = n->children[*i >> 4];
        return insert(child, first, last, i, LOW);
    }

    if (!n->children[*i & 15]) n->children[*i & 15] = nalloc().alloc();
    Node* const child = n->children[*i & 15];
    return insert(child, first, last, i + 1, HIGH);
}

std::size_t SymTrie::memory() const { return nalloc().used(); }

const L<J>& SymTrie::rank() const {
    static L<J> r;
    if (r.size() != positions.size()) {
        L<I> order;
        order.reserve(positions.size());
        preorder(&root, [&](S::rep s){ order.push_back(I(s)); });
        r = iota(order.size());
        std::stable_sort(r.begin(), r.end(), [&](J x, J y){ return order[x] < order[y]; });
    }
    return r;
}

H operator<<(H h, const SymTrie& t) {
    L<X> s;
    return print(h, t, &t.root, s);
}
