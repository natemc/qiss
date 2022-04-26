#pragma once

#ifdef __cplusplus

#include <cstdint>
#include <utility>

[[nodiscard]] std::pair<void*, uint64_t> qiss_alloc     (uint64_t bytes);
[[nodiscard]] uint64_t                   qiss_alloc_size(const void* p);
[[nodiscard]] uint64_t                   qiss_allocated ();
extern "C" {  void                       qiss_free      (void* p); }
[[nodiscard]] std::pair<void*, uint64_t> qiss_grow      (void* p, uint64_t to_bytes);
              void                       qiss_print();

// This guarantees that the allocator outlives all users.
// See https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter
namespace qiss_alloc_detail {
    static const struct QissAllocInit { QissAllocInit(); ~QissAllocInit(); } init;
}

#else

#include <stddef.h>

void* qiss_malloc (size_t);
void* qiss_realloc(void* p, size_t);
void  qiss_free   (void* p);

#endif
