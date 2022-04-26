#pragma once

#include <buddy_allocator.h>
#include <cstdint>

struct VisibleBuddyAllocator {
    VisibleBuddyAllocator();
    ~VisibleBuddyAllocator();
    VisibleBuddyAllocator(const VisibleBuddyAllocator&)            = delete;
    VisibleBuddyAllocator& operator=(const VisibleBuddyAllocator&) = delete;

    [[nodiscard]] std::pair<void*, uint64_t> alloc(uint64_t size);
    void                                     free (void* p);
    [[nodiscard]] std::pair<void*, uint64_t> grow (void* p, uint64_t new_size);
    [[nodiscard]] uint64_t                   size (const void* p)              const;
    [[nodiscard]] uint64_t                   used ()                           const;

    void print() const;

    typedef BuddyAllocator::Block    Block;
    typedef BuddyAllocator::bucket_t bucket_t;
    uint64_t bucket_to_bytes(bucket_t bucket) const { return ba.bucket_to_bytes(bucket); }
    Block*   header         (void*    p     )       { return ba.header(p); }

    BuddyAllocator ba;

private:
    struct Alloc { uint64_t i; void* p; uint64_t sz; }* allocs;
    uint64_t allocs_sz;
};
