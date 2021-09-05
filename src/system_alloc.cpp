#include <system_alloc.h>
#include <bits.h>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>

namespace {
    std::size_t g_heap;

    void default_oom_handler() {
        [[maybe_unused]] const ssize_t written = write(2, "'wsfull", 7);
        exit(1);
    }

    oom_handler_t g_oom_handler = default_oom_handler;

    void* alloc_or_die(std::size_t bytes) {
        assert(0 < bytes);
        void* const addr   = nullptr;
        const int   fd     = -1;
        const off_t offset = 0;
        void* const m      = mmap(addr, bytes,
            PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, fd, offset);
        if (m == MAP_FAILED) {
            g_oom_handler();
            exit(1);
        }
        return m;
    }
}

void* alloc_region(std::size_t bytes) {
    assert(0 < bytes);
    g_heap += bytes;
    return alloc_or_die(bytes);
}

// With this, plus putting the in use bit, bucket, and region bucket into
// Object's m member, we could eliminate the need for Header's meta, because we
// could compute the region base.  This would save 8 bytes per list.  Because
// 6 of those 8 bytes typically have the same content for all lists, I'm
// tempted.  However, there has to be a reason linux doesn't offer an
// mmap_aligned function, right?  There must be some downside (besides coupling
// the allocator to the Object representation)...
void* alloc_region_fully_aligned(std::size_t bytes) {
    assert(is_power_of_2(bytes));

    if (bytes <= system_page_size()) return alloc_region(bytes);

    // Alloc double the amount requested, find the address in the range
    // allocated that is a multiple of bytes, and then lop off the extra.
    void* const       a        = alloc_or_die(bytes * 2);
    char* const       m        = bitcast<char*>((bitcast<std::uintptr_t>(a) + bytes - 1) & -bytes);
    const std::size_t leading  = std::size_t(m - static_cast<char*>(a));
    const std::size_t trailing = bytes - leading;
    if (leading ) munmap(a        , leading);
    if (trailing) munmap(m + bytes, trailing);
        
    g_heap += bytes;
    return m;
}

std::size_t bytes_allocated() {
    return g_heap;
}

void free_region(void* p, std::size_t bytes) {
    assert(p);
    assert(bytes <= g_heap);
    [[maybe_unused]] const int rc = munmap(p, bytes);
    assert(rc == 0);
    g_heap -= bytes;
}

oom_handler_t oom_handler() {
    return g_oom_handler;
}

oom_handler_t set_oom_handler(oom_handler_t new_handler) {
    assert(new_handler);
    return std::exchange(g_oom_handler, new_handler);
}

std::size_t system_page_size() {
    // mmap aligns to sysconf(_SC_PAGESIZE)
    // We would like to know the page size at compile time, but it's possible
    // the executable could run on various hosts with different page sizes.
    static const auto size = sysconf(_SC_PAGESIZE);
    assert(0 < size);
    return std::size_t(size);
}
