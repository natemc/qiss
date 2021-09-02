#include <symtrie.h>

#include <algorithm>
#include <cassert>
#include <iasc.h>
#include <key.h>
#include <limits>
#include <numeric>
#include <primio.h>
#include <unistd.h>

SymTrie::Node::Node(): symbol(EMPTY) {
    std::fill(std::begin(children), std::end(children), nullptr);
}

SymTrie::Node::~Node() {
}

SymTrie::SymTrie() {
    root.symbol = 0;
    positions.push_back(I(0));
    symbols.push_back(C('\0'));
}

SymTrie::~SymTrie() {
    for (Node* c: root.children) { if (c) free(c); }
}

const char* SymTrie::c_str(S x) const {
    return reinterpret_cast<const char*>(&symbols[positions[S::rep(x)]]);
}

std::pair<bool, S> SymTrie::find(const Node* n, const char* s, hilo hl) const {
    return find(n, s, s + strlen(s), hl);
}

std::pair<bool, S>
SymTrie::find(const Node* n, const char* first, const char* last, hilo hl) const {
    if (!n) return {false, S(0)};
    if (first == last) return n->symbol == EMPTY? std::pair(false, S(0))
                                                : std::pair(true, S(n->symbol));
    return hl == HIGH? find(n->children[*first >> 4], first    , last, LOW)
                     : find(n->children[*first & 15], first + 1, last, HIGH);
}

void SymTrie::free(Node* n) {
    for (Node* c: n->children) { if (c) free(c); }
    nalloc.free(n);
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
        assert(find(&root, first, last, HIGH) == std::pair(true, S(n->symbol)));
        return std::pair(true, S(n->symbol));
    }

    if (hl == HIGH) {
        if (!n->children[*i >> 4]) n->children[*i >> 4] = nalloc.alloc();
        return insert(n->children[*i >> 4], first, last, i, LOW);
    }

    if (!n->children[*i & 15]) n->children[*i & 15] = nalloc.alloc();
    return insert(n->children[*i & 15], first, last, i + 1, HIGH);
}

std::size_t SymTrie::memory() const { return nalloc.used(); }

H SymTrie::print(H h, const SymTrie::Node* n, L<X>& s, int indent) const {
    constexpr const char digit[] = "0123456789abcdef";
    
    for (int i = 0; i < indent  ; ++i) h << ' ';
    for (int i = 0; i < s.size(); ++i) h << digit[X::rep(s[i]) & 15];
    if (n->symbol != SymTrie::EMPTY)
        h << " (" << n->symbol << "): `" << c_str(S(n->symbol));
    h << '\n';

    for (std::size_t i = 0; i < std::size(n->children); ++i) {
        if (n->children[i]) {
            s.push_back(X(X::rep(i)));
            print(h, n->children[i], s, indent + 2);
            s.pop_back();
        }
    }
    return h;
}

const L<J>& SymTrie::rank() const {
    static L<J> r;
    if (r.size() != positions.size()) {
        L<I> order;
        order.reserve(positions.size());
        preorder(&root, [&](S::rep s){ order.push_back(I(s)); });
        r = L<J>(iasc(order));
    }
    return r;
}

H operator<<(H h, const SymTrie& t) {
    L<X> s;
    return t.print(h, &t.root, s);
}
