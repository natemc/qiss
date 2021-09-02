#include <fixed_size_allocator.h>
#include <doctest.h>

TEST_CASE("FixedSizeAllocator smoke test") {
    FixedSizeAllocator<int> fsa;
    CHECK(0 == fsa.used());
    int* const p = fsa.alloc(42);
    CHECK(42 == *p);
    CHECK(sizeof(int) <= fsa.used());
    fsa.free(p);
    CHECK(0 == fsa.used());
}
