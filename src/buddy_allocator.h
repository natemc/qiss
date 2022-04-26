#pragma once

#include <cstdint>
#include <utility>

struct BuddyAllocator {
    constexpr BuddyAllocator() = default;
    ~BuddyAllocator();
    BuddyAllocator(const BuddyAllocator&)            = delete;
    BuddyAllocator& operator=(const BuddyAllocator&) = delete;

    [[nodiscard]] std::pair<void*, uint64_t> alloc(uint64_t size);
    void                                     free (void* p);
    [[nodiscard]] std::pair<void*, uint64_t> grow (void* p, uint64_t new_size);
    [[nodiscard]] uint64_t                   size (const void* p)              const;
    [[nodiscard]] uint64_t                   used ()                           const;
  
private:
    friend struct VisibleBuddyAllocator;

    using bucket_t = uint8_t;
    static constexpr uint64_t smallest_bucket_size = 32ull;
    static constexpr bucket_t buckets              = 48;
    static_assert(sizeof(void*) == sizeof(uint64_t));
    static_assert(8 <= buckets);

    // free_table has one free list per bucket size
    // Block size doubles with each bucket increment
    // Blocks from various region instances share these free lists.
    // bucket    blksz
    // -------------------------------------------------------------
    // 0         32
    // 1         64
    // 2         128
    // 3         256
    // 5         1024
    // 7         4096
    // 11        65536
    // 15        1MB
    // 21        64MB
    // 31        64GB
    // 41        64TB
    // 47        4PB
    struct Block {
        // bit  63   : 1 if in use, 0 if free
        // bits 62-12: region base address
        // bits 11-6 : region bucket (see table above)
        // bits 5-0  : block bucket (see table above)
        // UrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrRRRRRRbbbbbbb
        uint64_t meta;
        // How can we be smart about this?  We want to do the alignment
        // required for a particular call and not pay for max alignment
        // unless we really need it.  The problem is, we need to be able
        // to go from the address we handed out to its Block*.  If the
        // offset is not constant, we need to be able to lookup the offset.
        // If we static_assert(sizeof(std::max_align_t) == 8 || 16),
        // we only need 1 bit, but there's no good place to put it.
        // What if the required alignment were a template parameter?
        // Could we make that work without templatizing the whole thing?
        //union alignas(std::max_align_t) {
        union {
            char x[sizeof(Block*) * 2];
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
            struct {
                Block* next;
                Block* prev;
            };
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
        };

        Block(void* r, bucket_t bucket) { set_region(r, bucket); }
        bool        in_use       () const;
        void        mark_used    ();
        void        mark_unused  ();
        bucket_t    bucket       () const;
        Block*      set_bucket   (bucket_t bucket);
        void*       region       ();
        const void* region       () const;
        bucket_t    region_bucket() const;
        void        set_region   (void* r, bucket_t bucket);
    }* free_table[buckets] = {};

    uint64_t allocated = 0;

    void*        alloc_region   (uint64_t bytes);
    bucket_t     best_available (bucket_t at_least);
    uint64_t     bucket_to_bytes(bucket_t bucket) const;
    bucket_t     bytes_to_bucket(uint64_t bytes) const;
    void         cons           (Block* block);
    Block*       find_buddy     (Block* block);
    Block*       find_buddy     (Block* block, bucket_t bucket);
    void         free_block     (Block* block);
    void         free_region    (void* r, bucket_t bucket);
    Block*       header         (void* p);
    const Block* header         (const void* p) const;
    uint64_t     need           (uint64_t size);
    void         take_block     (Block* block);
    Block*       take_first     (bucket_t bucket);
};
