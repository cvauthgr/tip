#include <benchmark/benchmark.h>

#include "memory_pool.hpp"
#include "policies.hpp"

static void ptmalloc_bench(benchmark::State& state){
    // Allocate and construct a struct at place
    // with the new operator

    struct alignas(64) chest{
        int a = 67 ;
    };

    for(auto _ : state){
        auto ptr = new chest{67};
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;
        delete ptr ;
        benchmark::ClobberMemory() ;
    }

    state.SetBytesProcessed(state.iterations()*64) ;
}
BENCHMARK(ptmalloc_bench);

static void memory_pool_allocate_and_construct_bench(benchmark::State& state){
    // Here we will test the allocate_and_construct API
    tip::memory_pool<tip::unsafe , 4096> pool ;

    benchmark::DoNotOptimize(pool.acquire_internal_pool_start()) ;

    pool.cache_checkpoint() ;

    struct alignas(64) chest{
        int a = 67 ;
    };

    for(auto _ : state){
        auto ptr = pool.allocate_and_construct<chest>(67) ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;
        pool.revert() ;
    }

    state.SetBytesProcessed(state.iterations()*64) ;
}
BENCHMARK(memory_pool_allocate_and_construct_bench);
