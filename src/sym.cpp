#include <sym.h>
#include <doctest.h>
#include <new>
#include <prim.h>
#include <symtrie.h>
#include <type_traits>

namespace {
    typename std::aligned_storage<sizeof(SymTrie), alignof(SymTrie)>::type buf;
    SymTrie& symtrie = reinterpret_cast<SymTrie&>(buf);
    uint64_t init_counter;
}

namespace qiss_sym_detail {
    QissSymInit:: QissSymInit() { if (!init_counter++) new (&symtrie) SymTrie; }
    QissSymInit::~QissSymInit() { if (!--init_counter) symtrie.~SymTrie(); }
}

const char* c_str     (S x) { return symtrie.c_str(x); }
S           sym       (const char* cstr) { return symtrie.insert(cstr).second; }
S           sym       (const char* first, const char* last) {
    return symtrie.insert(first, last).second;
}
void        print_syms() { H(1) << symtrie; }
const L<J>& sym_rank  () { return symtrie.rank(); }
int32_t     sym_count () { return symtrie.size(); }

TEST_CASE("sym smoke test") {
    CHECK(S(0) == ""_s);
}
