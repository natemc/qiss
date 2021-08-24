#pragma once

#include <adverb.h>
#include <exception.h>
#include <l.h>

bool        operator==(const L<C>& x, const char* y);
inline bool operator==(const char* x, const L<C>& y) { return y == x; }
inline bool operator!=(const L<C>& x, const char* y) { return !(x == y); }
inline bool operator!=(const char* x, const L<C>& y) { return !(x == y); }

L<C>& operator<<(L<C>& buf, char           x);
L<C>& operator<<(L<C>& buf, const char*    x);
L<C>& operator<<(L<C>& buf, const void*    x);
L<C>& operator<<(L<C>& buf, int32_t        x);
L<C>& operator<<(L<C>& buf, int64_t        x);
L<C>& operator<<(L<C>& buf, uint32_t       x);
L<C>& operator<<(L<C>& buf, uint64_t       x);
L<C>& operator<<(L<C>& buf, std::size_t    x);
L<C>& operator<<(L<C>& buf, const L<C>&    x);
L<C>& operator<<(L<C>& buf, B              x);
L<C>& operator<<(L<C>& buf, C              x);
L<C>& operator<<(L<C>& buf, D              x);
L<C>& operator<<(L<C>& buf, F              x);
L<C>& operator<<(L<C>& buf, H              x);
L<C>& operator<<(L<C>& buf, I              x);
L<C>& operator<<(L<C>& buf, J              x);
L<C>& operator<<(L<C>& buf, S              x);
L<C>& operator<<(L<C>& buf, T              x);
L<C>& operator<<(L<C>& buf, X              x);
L<C>& operator<<(L<C>& buf, const Object*  x);
L<C>& operator<<(L<C>& buf, Adverb         x);
L<C>& operator<<(L<C>& buf, Opcode         x);
L<C>& operator<<(L<C>& buf, AO             x);
L<C>& operator<<(L<C>& buf, std::pair<const char*, const char*> x);
L<C>& operator<<(L<C>& buf, std::pair<const C*, const C*> x);
L<C>& escape(L<C>& buf, C x);

L<C> cstr2lc(const char* x);
Exception lc2ex(const L<C>& x);
L<C> ltrim(L<C> buf);
L<C> rtrim(L<C> buf);
L<C> trim (L<C> buf);
inline index_t width(const L<C>& x) { return x.size(); }
