#include <qiss_alloc.h>
#include <cstddef>
#include <new>

#if 0

#include <cstdlib>
#include <map>

namespace {
    using Allocs = std::map<const void*, size_t>;
    typename std::aligned_storage<sizeof(Allocs), alignof(Allocs)>::type buf;
    Allocs& allocs = reinterpret_cast<Allocs&>(buf);
    uint64_t init_counter;
}

namespace qiss_alloc_detail {
    QissAllocInit:: QissAllocInit() { if (init_counter++) new (&allocs) Allocs; }
    QissAllocInit::~QissAllocInit() { if (--init_counter) allocs.~Allocs(); }
}

std::pair<void*, uint64_t> qiss_alloc(uint64_t bytes) {
    void* const p = malloc(bytes);
    if (!p) return {p, 0ull};
    allocs[p] = bytes;
    return {p, bytes};
}

uint64_t qiss_alloc_size(const void* p) {
    return allocs[p];
}

uint64_t qiss_allocated() {
    return 0;
}

std::pair<void*, uint64_t> qiss_grow(void* p, uint64_t sz) {
    return sz <= qiss_alloc_size(p)? std::pair(p, sz) : std::pair(nullptr, 0ull);
}

extern "C" {
    void qiss_free(void* p) {
        if (p) {
            allocs.erase(p);
            free(p);
        }
    }
    void* qiss_malloc(size_t sz) { return malloc(sz); }
    void* qiss_realloc(void* p, size_t sz) { return realloc(p, sz); }
}

#else

#include <buddy_allocator.h>
#include <cstring>

namespace {
    using Alloc = BuddyAllocator;
    typename std::aligned_storage<sizeof(Alloc), alignof(Alloc)>::type buf;
    Alloc& lalloc = reinterpret_cast<Alloc&>(buf);
    uint64_t init_counter;
}

namespace qiss_alloc_detail {
    QissAllocInit:: QissAllocInit() { if (!init_counter++) new (&lalloc) Alloc; }
    QissAllocInit::~QissAllocInit() { if (!--init_counter) lalloc.~Alloc(); }
}

#ifdef DOCTEST_CONFIG_DISABLE
// For reasons that are not obvious from reading doctest.h, doctest requires
// new to return a 16-byte aligned pointer. Currently, qiss_alloc returns
// 8-byte aligned pointers. That could be changed, but it would be significant.
void* operator new   (std::size_t sz) { return qiss_alloc(sz).first; }
void  operator delete(void* p)              noexcept { qiss_free(p); }
void  operator delete(void* p, std::size_t) noexcept { qiss_free(p); }
#endif

uint64_t qiss_allocated() {
    return lalloc.used();
}

uint64_t qiss_alloc_size(const void* p) {
    return lalloc.size(p);
}

std::pair<void*, uint64_t> qiss_alloc(uint64_t bytes) {
    return lalloc.alloc(bytes);
}

std::pair<void*, uint64_t> qiss_grow(void* p, uint64_t to_bytes) {
    return lalloc.grow(p, to_bytes);
}

extern "C" {
    void* qiss_malloc(size_t sz) {
        return lalloc.alloc(sz).first;
    }

    void qiss_free(void*  p ) {
        if (p) lalloc.free(p);
    }

    void* qiss_realloc(void* p, size_t sz) {
        if (!p) return lalloc.alloc(sz).first;
        void* const r = lalloc.grow(p, sz).first;
        if (r) return r;
        void* const n = memcpy(lalloc.alloc(sz).first, p, qiss_alloc_size(p));
        lalloc.free(p);
        return n;
    }
}

#endif
