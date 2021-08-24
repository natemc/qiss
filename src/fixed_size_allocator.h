#pragma once

#include <bit>
#include <bits.h>
#include <cassert>
#include <cstddef>
#include <linked_list_len.h>
#include <memory>
#include <system_alloc.h>
#include <utility>

template <class X>
struct FixedSizeAllocator {
    FixedSizeAllocator(std::size_t pg_size = 0):
        full(nullptr),
        non_full(nullptr),
        page_size(pg_size? pg_size : system_page_size()),
        allocated(0)
    {
        assert(is_power_of_2(page_size));
        assert(sizeof(Page) + sizeof(Block) <= page_size);
    }
    FixedSizeAllocator(const FixedSizeAllocator&) = delete;
    FixedSizeAllocator& operator=(const FixedSizeAllocator&) = delete;
    ~FixedSizeAllocator() { assert(!full); assert(!non_full); }

    template <class... A>
    X* alloc(A&&... args) {
        if (!non_full) alloc_page();
        Page*  const p = non_full;
        Block* const b = p->free;
        p->free = b->next;
        ++p->n;
        if (!p->free) move_page(p, &non_full, &full);
        allocated += sizeof(Block);
        return new (&b->x) X{std::forward<A>(args)...};
    }

    void free(X* x) {
        Block* const b = reinterpret_cast<Block*>(x);
        x->~X();
        Page*  const p = page_from_block(b);
        if (!--p->n) {
            return_page(p);
        } else {
            if      (!p->free)           move_page(p, &full    , &non_full);
            else if (p->n < non_full->n) move_page(p, &non_full, &non_full);
            b->next = p->free;
            p->free = b;
        }
        allocated -= sizeof(Block);
    }

    std::size_t used() const { return allocated; }

/*    H dump(H h) {
        auto dump_free_blocks = [](H h, Page* p) {
            h << "    " << p << ": " << linked_list_len(p->free)
              << " blocks free\n";
        };
        for (auto p = full    ; p; p = p->next) dump_free_blocks(h, p);
        for (auto p = non_full; p; p = p->next) dump_free_blocks(h, p);
        return h;
    }*/

private:
    union Block {
        Block() = default;
        ~Block() = default;

        X      x   ;
        Block* next;
    };

    struct Page {
        Page*    next;
        Page*    prev;
        Block*   free;
        uint32_t n;
        uint32_t reserved;
        // Consider using the reserved bytes for a num_init variable used to
        // spread out the cost of initializing the list of free blocks (the
        // loop in alloc_page).  See
        // http://www.thinkmind.org/download.php?articleid=computation_tools_2012_1_10_80006.
        // Given the unavoidable system call, it may actually be worse unless
        // we use a much bigger page size.
    };

    Page* alloc_page() {
        void* const region = alloc_region_fully_aligned(page_size);
        Page* const p      = new (region) Page;
        assert(p == region); // page_from_block assumes this
        void*       ptr    = p + 1;
        std::size_t sz     = page_size - offset(region, ptr);
        void* const first  = std::align(alignof(Block), sizeof(Block), ptr, sz);
        assert(first);
        Block*     prev    = static_cast<Block*>(first);
        ptr                = prev + 1;
        sz                -= sizeof(Block);
        while (std::align(alignof(Block), sizeof(Block), ptr, sz)) {
            Block* const next  = static_cast<Block*>(ptr);
            prev->next         = next;
            prev               = next;
            ptr                = next + 1;
            sz                -= sizeof(Block);
        }
        assert(!prev->next);

        p->free       = static_cast<Block*>(first);
        p->n          = 0;
        p->prev       = nullptr;
        p->next       = non_full;
        non_full      = p;
        return p;
    }

    // Move p from (the list *from) to the head of (the list *to).
    void move_page(Page* p, Page** from, Page** to) {
        assert(*from && !*to || !(*to)->prev);
        if (*from == p) *from = p->next;
        if (p->prev) p->prev->next = p->next;
        if (p->next) p->next->prev = p->prev;
        p->prev = nullptr;
        p->next = *to;
        if (*to) (*to)->prev = p;
        *to = p;
    }

    static std::size_t offset(const void* base, const void* p) {
        return std::size_t(static_cast<const char*>(p) - static_cast<const char*>(base));
    }

    Page* page_from_block(Block* b) {
        return bitcast<Page*>(bitcast<std::uintptr_t>(b) & -page_size);
    }

    void return_page(Page* p) {
        if (p->next)                  p->next->prev = p->prev;
        if (p->prev)                  p->prev->next = p->next;
        else { assert(non_full == p); non_full      = p->next; }
        p->~Page();
        free_region(p, page_size);
    }

    Page*             full;
    Page*             non_full;
    const std::size_t page_size;
    std::size_t       allocated;
};
