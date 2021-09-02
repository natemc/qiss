#pragma once

#include <cstddef>
#include <cstdint>
#include <prim.h>
#include <utility>

const struct Truncate {} TRUNCATE;

H hopen(const char* path);
H hopen(const char* path, Truncate);
void hclose(H h);

H escape(H h, C x);
H flush(H h);

H operator<<(H h, H(*)(H));
H operator<<(H h, B x);
H operator<<(H h, C x);
H operator<<(H h, D x);
H operator<<(H h, F x);
H operator<<(H h, H x);
H operator<<(H h, I x);
H operator<<(H h, J x);
H operator<<(H h, S x);
H operator<<(H h, T x);
H operator<<(H h, X x);
H operator<<(H h, std::pair<const C*, const C*> x);

H operator<<(H h, char        x);
H operator<<(H h, uint8_t     x);
H operator<<(H h, int32_t     x);
H operator<<(H h, int64_t     x);
H operator<<(H h, uint32_t    x);
H operator<<(H h, uint64_t    x);
H operator<<(H h, std::size_t x);
H operator<<(H h, double      x);
H operator<<(H h, const void* x);
H operator<<(H h, const char* x);
H operator<<(H h, std::pair<const char*, const char*> x);
