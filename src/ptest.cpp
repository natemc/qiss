#include <benchmark/benchmark.h>
#include <fixed_size_allocator.h>
#include <object.h>
#include <vector>

namespace {
    void benchmark_fsa(benchmark::State& state) {
        std::vector<Object*> v(100);
        FixedSizeAllocator<Object> fsa;
        for (auto _: state) {
            for (Object*& p: v) p = fsa.alloc();
            for (Object*& p: v) fsa.free(p);
        }
    }
    BENCHMARK(benchmark_fsa);

    void benchmark_new(benchmark::State& state) {
        std::vector<Object*> v(100);
        for (auto _: state) {
            for (Object*& p: v) p = new Object;
            for (Object*& p: v) delete p;
        }
    }
    BENCHMARK(benchmark_new);
}

BENCHMARK_MAIN();
