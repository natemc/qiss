#include <buddy_allocator.h>
#include <algorithm>
#include <bits.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <doctest.h>
#include <new>
//#include <primio.h>
#include <system_alloc.h>
#include <unistd.h>
#include <utility>

/*template <class X> H print_list(H h, X* head) {
    h << head;
    if (head) {
        assert(!head->prev);
        X* prev = head;
        for (head = head->next; head; head = head->next) {
            assert(head->prev == prev);
            h << " -> " << head;
            prev = head;
        }
    }
    return h << '\n';
}*/

BuddyAllocator::~BuddyAllocator() {
    if (allocated) {
        char buf[64];
        std::size_t i = 0;
        for (uint64_t p = allocated; p; p /= 10, ++i) buf[i] = char('0' + p % 10);
        std::reverse(buf, buf + i);
        [[maybe_unused]] ssize_t written = write(2, buf, i);
        const char msg[] = " bytes leaked\n";
        written = write(2, msg, sizeof msg);
    }
    assert(allocated == 0);
}

std::pair<void*, uint64_t> BuddyAllocator::alloc(uint64_t size) {
    const uint64_t bytes = need(size);
    if (bucket_to_bytes(buckets - 1) < bytes) {
        return {nullptr, 0ull};
    }
    const bucket_t bucket = bytes_to_bucket(bytes);
//    print_list(H(1) << "|||| free list " << int(bucket) << ": ",
//               free_table[bucket]) << flush;
    Block* block = take_first(bucket);
    if (!block) {
        bucket_t b = best_available(bucket);
        if (b < buckets) {
            block = take_first(b);
        } else {
            const bucket_t MIN_ALLOC_BUCKET = 21; // 64MB
            b              = std::max(bucket, MIN_ALLOC_BUCKET);
            //void* const  r = alloc_region_fully_aligned(bucket_to_bytes(b));
            void* const  r = alloc_region(bucket_to_bytes(b));
            if (!r) return {nullptr, 0ull};
            block          = new (r) Block;
            block->set_region(r, b);
        }

        void* const    region        = block->region();
        const bucket_t region_bucket = block->region_bucket();
        while (b > bucket) { // split selected/allocated chunk onto free lists
            --b;
            char* const   p     = reinterpret_cast<char*>(block) + bucket_to_bytes(b);
            Block* const right = new (p) Block;
            right->set_bucket(b);
            right->set_region(region, region_bucket);
            cons(right);
        }
        block->set_bucket(bucket);
    }
    assert(block->bucket() == bucket);
    block->mark_used();
    const uint64_t allocated_bytes = bucket_to_bytes(bucket);
    allocated += allocated_bytes;
//    H(1) << "LIST " << block << "\trequested: "
//              << std::setw(4) << size
//              << "\tneeded: " << bytes
//              << "\tbucket: " << int(block->bucket())
//              << "\tbucket size: " << std::setw(4) << allocated_bytes
//              << "\tmeta: " << std::hex << block->meta << std::dec
//              << "\tx: " << (&block->x) << '\n' << flush;
//    print_list(H(1) << "//// free list " << int(bucket) << ": ",
//               free_table[bucket]) << flush;
    return {block->x, allocated_bytes - offsetof(Block, x)};
}

void BuddyAllocator::free(void* p) {
    Block* const block = header(p);
    assert(block->in_use());
//    H(1) << "FREE " << p << "\tbucket: " << int(block->bucket())
//              << "\tmeta: " << std::hex << block->meta << std::dec
//              << '\n' << flush;
    allocated -= bucket_to_bytes(block->bucket());
    free_block(block);

//    print_list(H(1) << "---- free list " << int(bucket) << ": ",
//               free_table[bucket]) << flush;
}

std::pair<void*, uint64_t> BuddyAllocator::grow(void* p, uint64_t new_size) {
    Block* const   block           = header(p);
    const uint64_t request         = need(new_size);
    const bucket_t original_bucket = block->bucket();
    bucket_t       bucket          = original_bucket;
    Block* buddies[buckets];
    while (bucket < buckets && bucket_to_bytes(bucket) < request) {
        Block* const buddy = find_buddy(block, bucket);
        if (buddy->in_use() || buddy->bucket() != bucket || buddy < block)
            return {nullptr, 0};
        buddies[bucket++] = buddy;
    }
    if (bucket == buckets) return {nullptr, 0};
    for (bucket_t b = block->bucket(); b < bucket; ++b) {
        take_block(buddies[b]);
        buddies[b]->~Block();
    }
    block->set_bucket(bucket);
    const uint64_t allocated_bytes = bucket_to_bytes(bucket);
    allocated += allocated_bytes - bucket_to_bytes(original_bucket);
    return {block->x, allocated_bytes - offsetof(Block, x)};
}

uint64_t BuddyAllocator::size(const void* p) const {
    return bucket_to_bytes(header(p)->bucket()) - offsetof(Block, x);
}

uint64_t BuddyAllocator::used() const {
    return allocated;
}

////////////////////////////////////////////////////////////////////////
// Private methods
////////////////////////////////////////////////////////////////////////

void* BuddyAllocator::alloc_region(uint64_t bytes) {
    void* const p = ::alloc_region(bytes);
    assert((bitcast<std::uintptr_t>(p) & 0x8000'0000'0000'0FFF) == 0);
//    H(1) << "LIST REGION " << p << " (" << bytes << ")\n" << flush;
    return p;
}

BuddyAllocator::bucket_t BuddyAllocator::best_available(bucket_t at_least) {
    bucket_t b = at_least;
    while (b < buckets && !free_table[b]) ++b;
    return b;
}

uint64_t BuddyAllocator::bucket_to_bytes(bucket_t bucket) const {
    assert(bucket < buckets);
    return smallest_bucket_size << bucket;
}

BuddyAllocator::bucket_t BuddyAllocator::bytes_to_bucket(uint64_t bytes) const {
    assert(0 < bytes);
//    H(1) << "bytes: " << bytes << "\tclz(bytes - 1): " << clz(bytes - 1)
//              << "\t59 - clz(bytes - 1): " << (59 - clz(bytes - 1)) << '\n' << flush;
    const bucket_t bucket(bucket_t(std::max(0, 59 - clz(bytes - 1))));
    assert(bucket < buckets);
    assert(bucket == 0 || bucket_to_bytes(bucket - 1) < bytes);
    assert(bytes <= bucket_to_bytes(bucket));
    return bucket;
}

void BuddyAllocator::cons(Block* block) {
    const bucket_t bucket = block->bucket();
    block->next           = free_table[bucket];
    if (block->next) block->next->prev = block;
    block->prev           = nullptr;
    free_table[bucket]    = block;
}

BuddyAllocator::Block* BuddyAllocator::find_buddy(Block* block) {
    return find_buddy(block, block->bucket());
}

BuddyAllocator::Block* BuddyAllocator::find_buddy(Block* block, bucket_t bucket) {
    const uint64_t    bytes  = bucket_to_bytes(bucket);
    char* const       b      = static_cast<char*>(block->region());
    char* const       x      = reinterpret_cast<char*>(block);
    assert(b <= x && x < b + bucket_to_bytes(block->region_bucket()));
    const std::size_t offset = std::size_t(x - b);
    void* const y = ctz(offset) < ctz(offset + bytes)? x - bytes : x + bytes;
    Block* const      buddy  = static_cast<Block*>(y);
    assert(std::as_const(buddy)->region() == b);
    return buddy;
}

void BuddyAllocator::free_block(Block* block) {
    void* const    r = block->region();
    const bucket_t b = block->region_bucket();
//    H(1) << "FREE " << block << "\tbucket: " << int(block->bucket())
//              << "\tregion: " << r << "\tregion bucket: " << int(b) << '\n' << flush;
    if (block == r && block->bucket() == b) {
        block->~Block();
        free_region(r, b);
    } else {
        block->mark_unused();
        const bucket_t bucket = block->bucket();
        Block* const   buddy  = find_buddy(block);
        if (buddy->bucket() != bucket || buddy->in_use()) {
//            H(1) << "BUDDY IN USE block: " << block
//                 << "\tbucket: " << int(bucket)
//                 << "\tbucket size: " << bucket_to_bytes(bucket)
//                 << "\n             buddy: " << buddy
//                 << "\tbucket: " << int(buddy->bucket())
//                 << "\tbucket size: " << bucket_to_bytes(buddy->bucket())
//                 << '\n' << flush;
            cons(block);
        } else {
//            H(1) << "BUDDY FREE block: " << block << "\tbuddy: " << buddy
//                 << "\tbucket: " << int(bucket) << "\tbucket size: "
//                 << bucket_to_bytes(bucket) << '\n' << flush;
            take_block(buddy);
            Block* const coalesced = std::min(block, buddy);
            std::max(block, buddy)->~Block();
            coalesced->set_bucket(bucket + 1);
            if (bucket + 1 < b) {
                free_block(coalesced);
            } else {
                assert(coalesced == r);
                coalesced->~Block();
                free_region(r, b);
            }
        }
    }
}

void BuddyAllocator::free_region(void* r, bucket_t bucket) {
//    H(1) << "FREE REGION " << r << ' ' << bucket_to_bytes(bucket) << '\n' << flush;
    ::free_region(r, bucket_to_bytes(bucket));
}

BuddyAllocator::Block* BuddyAllocator::header(void* p) {
    void* const v = static_cast<char*>(p) - offsetof(Block, x);
    return static_cast<Block*>(v);
}

const BuddyAllocator::Block* BuddyAllocator::header(const void* p) const {
    const void* const v = static_cast<const char*>(p) - offsetof(Block, x);
    return static_cast<const Block*>(v);
}

uint64_t BuddyAllocator::need(uint64_t size) {
    return size + offsetof(Block, x);
}

void BuddyAllocator::take_block(Block* block) {
    if (block->next) block->next->prev = block->prev;
    assert(block->prev || free_table[block->bucket()] == block);
    (block->prev? block->prev->next : free_table[block->bucket()]) = block->next;
}

BuddyAllocator::Block* BuddyAllocator::take_first(bucket_t bucket) {
    Block* const block = free_table[bucket];
    if (block) {
        assert(block->bucket() == bucket);
        assert(!block->prev);
        if (block->next) block->next->prev = nullptr;
        free_table[bucket] = block->next;
    }
    return block;
}

////////////////////////////////////////////////////////////////////////
// Block methods
////////////////////////////////////////////////////////////////////////

bool BuddyAllocator::Block::in_use      () const { return meta & 1ull<<63; }
void BuddyAllocator::Block::mark_used   ()       { meta |= 1ull<<63; }
void BuddyAllocator::Block::mark_unused ()       { meta &= ~(1ull<<63); }

BuddyAllocator::bucket_t BuddyAllocator::Block::bucket() const {
    return meta & 63;
}
void BuddyAllocator::Block::set_bucket(bucket_t bucket) {
    assert(bucket < buckets);
    meta = (meta & uint64_t(-64)) | bucket;
}

void* BuddyAllocator::Block::region() {
    const std::uintptr_t r = meta & 0x7FFF'FFFF'FFFF'F000;
    void* p;
    memcpy(&p, &r, sizeof p);
    return p;
}

const void* BuddyAllocator::Block::region() const {
    return const_cast<Block*>(this)->region();
}

BuddyAllocator::bucket_t BuddyAllocator::Block::region_bucket() const {
    return (meta & 0x0000'0000'0000'0FC0) >> 6;
}

void BuddyAllocator::Block::set_region(void* r, bucket_t bucket) {
    uint64_t m;
    memcpy(&m, &r, sizeof m);
    meta = (meta & (1ull<<63)) | m | (uint64_t(bucket) << 6) | (meta & 63);
}

TEST_CASE("buddy_allocator smoke test") {
    BuddyAllocator ba;
    const auto [p, sz] = ba.alloc(1024);
    CHECK(1024 <= sz);
    CHECK(sz == ba.size(p));
    CHECK(sz <= ba.used());
    ba.free(p);
    CHECK(0 == ba.used());
}
