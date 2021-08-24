#include <qiss_alloc.h>
#include <buddy_allocator.h>
#include <cstdio>
#include <new>

namespace {
    union Storage { ~Storage() {} BuddyAllocator alloc; } storage{};
    BuddyAllocator& lalloc = storage.alloc;
    uint64_t init_counter;
}

namespace qiss_alloc_detail {
    QissAllocInit::QissAllocInit() {
        if (init_counter++ == 0) new (&lalloc) BuddyAllocator;
    }

    QissAllocInit::~QissAllocInit() {
        if (--init_counter == 0) lalloc.~BuddyAllocator();
    }
}

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
