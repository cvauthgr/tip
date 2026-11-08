#include <benchmark/benchmark.h>
#include <cstddef>

#include "allocator.hpp"
#include "policies.hpp"

static void ptmalloc_bench(benchmark::State& state){
    struct  megabyte_256{
        char data[256*1024*1024] ;
    };

    for(auto _ : state){
        auto ptr = new megabyte_256 ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;
        state.PauseTiming() ;
        delete ptr ;
        state.ResumeTiming() ;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(ptmalloc_bench)->MinTime(5.0)
                         ->Repetitions(5)
                         ->ReportAggregatesOnly(true);

static void allocator_single_allocate(benchmark::State& state){

    constexpr std::size_t one_gigabyte = 1024*1024*1024 ;

    tip::allocator<tip::unsafe , one_gigabyte> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct  megabyte_256{
        char data[256*1024*1024] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<megabyte_256>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;
        allocator.reclaim_memory() ;
        benchmark::ClobberMemory() ;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(allocator_single_allocate)->MinTime(5.0)
                                    ->Repetitions(5)
                                    ->ReportAggregatesOnly(true);
