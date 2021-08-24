#pragma once

#include <cstdint>
//#include <l.h>
#include <prim.h>

const char*    c_str     (S x);
S              sym       (const char* s);
S              sym       (const char* first, const char* last);
inline S       sym       (const C*    first, const C*    last) {
    return sym(reinterpret_cast<const C::rep*>(first),
               reinterpret_cast<const C::rep*>(last));
}
//inline S       sym       (const L<C>& s) { return sym(s.begin(), s.end()); }
void           print_syms();
int32_t        sym_count ();
//const L<J>&    sym_rank  ();
