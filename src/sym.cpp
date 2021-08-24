#include <sym.h>
#include <prim.h>
#include <symtrie.h>

namespace {
    SymTrie symtrie;
}

const char* c_str     (S x) { return symtrie.c_str(x); }
S           sym       (const char* cstr) { return symtrie.insert(cstr).second; }
S           sym       (const char* first, const char* last) {
    return symtrie.insert(first, last).second;
}
void        print_syms() { H(1) << symtrie; }
const L<J>& sym_rank  () { return symtrie.rank(); }
int32_t     sym_count () { return symtrie.size(); }
