#include <qiss_alloc.h>
#include <algorithm>
#include <cstddef>
#include <new>
#include <unistd.h>

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

#include <visible_buddy_allocator.h>
#include <cstring>

namespace {
    using Allocator = VisibleBuddyAllocator;
    typename std::aligned_storage<sizeof(Allocator), alignof(Allocator)>::type buf;
    Allocator& lalloc = reinterpret_cast<Allocator&>(buf);
    uint64_t init_counter;
}

namespace qiss_alloc_detail {
    QissAllocInit:: QissAllocInit() { if (!init_counter++) new (&lalloc) Allocator; }
    QissAllocInit::~QissAllocInit() { if (!--init_counter) lalloc.~Allocator(); }
}

/*
#ifdef DOCTEST_CONFIG_DISABLE
// For reasons that are not obvious from reading doctest.h, doctest requires
// new to return a 16-byte aligned pointer. Currently, qiss_alloc returns
// 8-byte aligned pointers. That could be changed, but it would be significant.
void* operator new   (std::size_t sz) { return qiss_alloc(sz).first; }
void  operator delete(void* p)              noexcept { qiss_free(p); }
void  operator delete(void* p, std::size_t) noexcept { qiss_free(p); }
#endif
*/

uint64_t qiss_allocated() {
    return lalloc.used();
}

uint64_t qiss_alloc_size(const void* p) {
    return lalloc.size(p);
}

std::pair<void*, uint64_t> qiss_alloc(uint64_t bytes) {
    auto [p, sz] = lalloc.alloc(bytes);
    if (!p) {
        const char msg[] = "oom allocating ";
        [[maybe_unused]] ssize_t written = write(2, msg, sizeof msg - 1);
        char buf[64];
        std::size_t i = 0;
        for (uint64_t b = bytes; b; b /= 10, ++i) buf[i] = char('0' + b % 10);
        std::reverse(buf, buf + i);
        written = write(2, buf, i);
        const char msg2[] = " bytes\n";
        written = write(2, msg2, sizeof msg2 - 1);
        exit(1);
    }
    return std::pair{p, sz};
}

std::pair<void*, uint64_t> qiss_grow(void* p, uint64_t to_bytes) {
    return lalloc.grow(p, to_bytes);
}

void qiss_print() {
    lalloc.print();
}

void qiss_print_old() {
    lalloc.print_old();
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
