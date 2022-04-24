#pragma once

#include <cstddef>
#include <cstdint>
#include <l.h>
#include <prim.h>

namespace qiss_sym_detail {
    static const struct QissSymInit { QissSymInit(); ~QissSymInit(); } init;
}

const char*  c_str       (S x);
S            sym         (const char* s);
S            sym         (const char* first, const char* last);
inline S     sym         (const C*    first, const C*    last) {
    return sym(reinterpret_cast<const C::rep*>(first),
               reinterpret_cast<const C::rep*>(last));
}
inline S     sym         (const L<C>& s) { return sym(s.begin(), s.end()); }
inline S     operator""_s(const char* s, std::size_t n) { return sym(s, s + n); }
void         print_syms  ();
int32_t      sym_count   ();
const L<J>&  sym_rank    ();
