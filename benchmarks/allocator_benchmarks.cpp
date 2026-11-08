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
                         ->Repetitions(2)
                         ->ReportAggregatesOnly(true);

static void allocator_single_allocate(benchmark::State& state){

    constexpr std::size_t one_gigabyte = 1024*1024*1024 ;

    tip::allocator<tip::default_policy , one_gigabyte> allocator ;

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
                                    ->Repetitions(2)
                                    ->ReportAggregatesOnly(true);;

static void allocator_default_policy(benchmark::State& state){
    tip::allocator<tip::default_policy , 1024*1024*32> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct big_struct{
        char data[1024*1024*16] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<big_struct>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_default_policy)->MinTime(5.0)
                                   ->Repetitions(2)
                                   ->ReportAggregatesOnly(true);;

static void allocator_unsafe_policy(benchmark::State& state){
    tip::allocator<tip::unsafe , 1024*1024*32> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct big_struct{
        char data[1024*1024*16] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<big_struct>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_unsafe_policy)->MinTime(5.0)
                                  ->Repetitions(2)
                                  ->ReportAggregatesOnly(true);
