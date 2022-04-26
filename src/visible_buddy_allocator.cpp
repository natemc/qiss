#include <algorithm>
#include <cstring>
#include <system_alloc.h>
#include <terminal_width.h>
#include <unistd.h>
#include <utility>
#include <visible_buddy_allocator.h>

#define L1(e) [&](auto&& x){ return e; }
#define L2(e) [&](auto&& x, auto&& y){ return e; }

namespace {
    const std::size_t MAX_ALLOCS = 1'000'000;

    const char dred   [] = "\033[30;48;5;1m";
    const char dgreen [] = "\033[30;48;5;2m";
    const char canvas [] = "\033[30;48;5;3m";
    const char dblue  [] = "\033[30;48;5;4m";
    const char purple [] = "\033[30;48;5;5m";
    const char aqua   [] = "\033[30;48;5;6m";
    const char gray   [] = "\033[30;48;5;7m";
    const char dgray  [] = "\033[30;48;5;8m";
    const char bred   [] = "\033[30;48;5;9m";
    const char bgreen [] = "\033[30;48;5;10m";
    const char byellow[] = "\033[30;48;5;11m";
    const char blue   [] = "\033[30;48;5;12m";
    const char pink   [] = "\033[30;48;5;13m";
    const char cyan   [] = "\033[30;48;5;14m";
    const char white  [] = "\033[30;48;5;15m";
    const char lgreen [] = "\033[30;48;5;193m";
    const char* colors[] = {
        dred, dgreen, canvas, dblue, purple, aqua, gray, dgray,
        bred, bgreen, byellow, blue, pink, cyan, white, lgreen,
    };
    const char reset  [] = "\033[0m";

    struct Region {
        const void* p;
        uint64_t    sz;
    };

    template <std::size_t N> void out(int fd, const char(&s)[N]) {
        [[maybe_unused]] const ssize_t written = write(fd, s, N - 1);
    }

    void out(int fd, const char* s) {
        [[maybe_unused]] const ssize_t written = write(fd, s, strlen(s));
    }

    int out(int fd, uint64_t x) {
        char buf[64];
        int i = 0;
        for (uint64_t p = x; p; p /= 10, ++i) buf[i] = char('0' + p % 10);
        std::reverse(buf, buf + i);
        [[maybe_unused]] ssize_t written = write(fd, buf, std::size_t(i));
        return i;
    }
}

VisibleBuddyAllocator::VisibleBuddyAllocator():
    allocs(static_cast<Alloc*>(alloc_region(sizeof(Alloc) * MAX_ALLOCS))),
    allocs_sz(0)
{
}

VisibleBuddyAllocator::~VisibleBuddyAllocator() {
    free_region(allocs, sizeof(Alloc) * MAX_ALLOCS);
}

std::pair<void*, uint64_t> VisibleBuddyAllocator::alloc(uint64_t size) {
    if (allocs_sz == MAX_ALLOCS) {
        const char buf[] = "Too many outstanding memory allocations for memory tracer\n";
        [[maybe_unused]] ssize_t written = write(2, buf, sizeof(buf));
        exit(1);
    }

    const auto [p, sz] = ba.alloc(size);
    allocs[allocs_sz] = {allocs_sz, p, sz};
    ++allocs_sz;
    return std::pair{p, sz};
}

void VisibleBuddyAllocator::free(void* p) {
    const auto it = std::find_if(allocs, allocs + allocs_sz, L1(x.p == p));
    if (it == allocs + allocs_sz) {
        const char buf[] = "free requested for unknown allocation\n";
        [[maybe_unused]] ssize_t written = write(2, buf, sizeof(buf));
        exit(1);
    } else {
        ba.free(p);
        std::copy(it + 1, allocs + allocs_sz--, it);
    }
}

std::pair<void*, uint64_t> VisibleBuddyAllocator::grow(void* p, uint64_t new_size) {
    return ba.grow(p, new_size);
}

void VisibleBuddyAllocator::print() const {
    if (allocs_sz == 0) return;
    std::sort(allocs, allocs + allocs_sz, L2(x.p < y.p));
    constexpr uint64_t MAX_REGIONS = 1;
    Region regions[MAX_REGIONS] = {};
    uint64_t regions_sz = 0;
    for (uint64_t i = 0; i < allocs_sz; ++i) {
        const Alloc& alloc = allocs[i];
        const Block* const block = ba.header(alloc.p);
        const auto it = std::find_if(regions, regions + regions_sz, L1(x.p == block->region()));
        if (it == regions + regions_sz) {
            if (regions_sz == MAX_REGIONS) {
                out(2, "Too many outstanding memory regions for memory tracer\n");
                return;
            }
            regions[regions_sz++] = {block->region(), ba.bucket_to_bytes(block->region_bucket())};
        }
    }
    if (regions_sz != 1) {
        out(2, "Too many outstanding memory regions for memory tracer\n");
        return;
    }

    const int   tw = terminal_width();
    const char* m  = static_cast<const char*>(regions->p); // memory offset
    const int   iw = 4; // index width
    size_t c       = 0; // color
    int x          = 0; // column
    for (uint64_t i = 0; i < allocs_sz; ++i) {
        const Alloc& alloc = allocs[i];
        const char* const p = static_cast<char*>(alloc.p) - 8;
        const char* const q = static_cast<char*>(alloc.p) + alloc.sz;
        for (; m < p; m += 8, ++x) out(1, " ");
        out(1, colors[c]);
        // Don't want to introduce this dependency directly if I can help it
        // const Object* const o = static_cast<const Object*>(alloc.p);
        const int n = out(1, alloc.i);
        for (int j = 0; j < iw - n; ++j) out(1, " ");
        x += iw;
        for (m += iw * 8; m < q; m += 8, ++x) out(1, " ");
        out(1, reset);
        c = (c + 1) % std::size(colors);
        x = x % tw;
    }
    for (; x < tw; ++x) out(1, " ");
    out(1, "\n");
}

uint64_t VisibleBuddyAllocator::size(const void* p) const {
    return ba.size(p);
}

uint64_t VisibleBuddyAllocator::used() const {
    return ba.used();
}
