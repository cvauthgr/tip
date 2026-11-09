#include <benchmark/benchmark.h>

#include <cstddef>
#include <memory_resource>

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
BENCHMARK(ptmalloc_bench)->MinTime(1)
                         ->Repetitions(15)
                         ->DisplayAggregatesOnly(true);

static void monotonic_buffer_resource(benchmark::State& state){

    struct megabyte_256{
        char data[256*1024*1024] ;
    };

    alignas(megabyte_256) static std::byte buffer[sizeof(megabyte_256) * 2];
    std::pmr::monotonic_buffer_resource source(buffer, sizeof(buffer), std::pmr::null_memory_resource());

    for(auto _ : state) {
        auto ptr = source.allocate(sizeof(megabyte_256), alignof(megabyte_256));
        benchmark::DoNotOptimize(ptr);
        benchmark::ClobberMemory();

        // Reset the internal bump pointer to the start of the buffer for the next iteration
        source.release();
    }

    state.SetItemsProcessed(state.iterations());

}
BENCHMARK(monotonic_buffer_resource)->MinTime(1)
                                    ->Repetitions(15)
                                    ->DisplayAggregatesOnly(true);

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
BENCHMARK(allocator_single_allocate)->MinTime(1)
                                    ->Repetitions(15)
                                    ->DisplayAggregatesOnly(true);

static void allocator_default_policy(benchmark::State& state){
    tip::allocator<tip::default_policy , 1024*1024*1024> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct megabyte_256{
        char data[1024*1024*256] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<megabyte_256>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_default_policy)->MinTime(1)
                                   ->Repetitions(15)
                                   ->DisplayAggregatesOnly(true);;

static void allocator_unsafe_policy(benchmark::State& state){
    tip::allocator<tip::unsafe , 1024*1024*1024> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct megabyte_256{
        char data[1024*1024*256] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<megabyte_256>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_unsafe_policy)->MinTime(1)
                                  ->Repetitions(15)
                                  ->DisplayAggregatesOnly(true);

static void ptmalloc_bench_touch_memory(benchmark::State& state){
    struct  megabyte_256{
        char data[256*1024*1024] ;
    };

    for(auto _ : state){
        auto ptr = new megabyte_256 ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        for(std::size_t i = {0uz} ; i < sizeof(*ptr) ; i += 4096){
            ptr->data[i] = 'a' ;
        }

        delete ptr ;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(ptmalloc_bench_touch_memory)->MinTime(1)
                                      ->Repetitions(15)
                                      ->DisplayAggregatesOnly(true);

static void monotonic_buffer_resource_touch_memory(benchmark::State& state){

    struct megabyte_256{
        char data[256*1024*1024] ;
    };

    alignas(megabyte_256) static std::byte buffer[sizeof(megabyte_256) * 2];
    std::pmr::monotonic_buffer_resource source(buffer, sizeof(buffer), std::pmr::null_memory_resource());

    for(auto _ : state) {
        auto ptr = static_cast<megabyte_256*>(source.allocate(sizeof(megabyte_256), alignof(megabyte_256)));
        benchmark::DoNotOptimize(ptr);
        benchmark::ClobberMemory();

        for(std::size_t i = {0uz} ; i < sizeof(*ptr) ; i += 4096){
            ptr->data[i] = 'a' ;
        }

        // Reset the internal bump pointer to the start of the buffer for the next iteration
        source.release();
    }

    state.SetItemsProcessed(state.iterations());

}
BENCHMARK(monotonic_buffer_resource_touch_memory)->MinTime(1)
                                                 ->Repetitions(15)
                                                 ->DisplayAggregatesOnly(true);

static void allocator_single_allocate_touch_memory(benchmark::State& state){

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

        for(std::size_t i = {0uz} ; i < sizeof(*ptr) ; i += 4096){
            ptr->data[i] = 'a' ;
        }

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(allocator_single_allocate_touch_memory)->MinTime(1)
                                                 ->Repetitions(15)
                                                 ->DisplayAggregatesOnly(true);

static void allocator_default_policy_touch_memory(benchmark::State& state){
    tip::allocator<tip::default_policy , 1024*1024*1024> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct megabyte_256{
        char data[1024*1024*256] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<megabyte_256>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        for(std::size_t i = {0uz} ; i < sizeof(*ptr) ; i += 4096){
            ptr->data[i] = 'a' ;
        }

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_default_policy_touch_memory)->MinTime(1)
                                                ->Repetitions(15)
                                                ->DisplayAggregatesOnly(true);;

static void allocator_unsafe_policy_touch_memory(benchmark::State& state){
    tip::allocator<tip::unsafe , 1024*1024*1024> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    struct megabyte_256{
        char data[1024*1024*256] ;
    };

    for(auto _ : state){
        auto ptr = allocator.allocate<megabyte_256>() ;
        benchmark::DoNotOptimize(ptr) ;
        benchmark::ClobberMemory() ;

        for(std::size_t i = {0uz} ; i < sizeof(*ptr) ; i += 4096){
            ptr->data[i] = 'a' ;
        }

        allocator.reclaim_memory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_unsafe_policy_touch_memory)->MinTime(1)
                                               ->Repetitions(15)
                                               ->DisplayAggregatesOnly(true);

static void ptmalloc_bench_L1D(benchmark::State& state){
    const std::size_t size  = static_cast<std::size_t>(state.range(0)) ;
    const std::size_t count = (1024*32) / size ;

    std::array<void*, (1024*32) / 64> pointers ;

    for(auto _ : state){
        for(std::size_t i = {0uz} ; i < count ; ++i){
            pointers[i] = ::operator new(size) ;
            benchmark::DoNotOptimize(pointers[i]) ;
        }
        benchmark::ClobberMemory() ;

        for(std::size_t i = {0uz} ; i < count ; ++i){
            ::operator delete(pointers[i]) ;
        }
        benchmark::ClobberMemory() ;
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count)) ;
}
BENCHMARK(ptmalloc_bench_L1D)->RangeMultiplier(4)
                         ->Range(64 , 16 * 1024)
                         ->MinTime(1)
                         ->Repetitions(15)
                         ->DisplayAggregatesOnly(true);

static void monotonic_buffer_resource_L1D(benchmark::State& state){
    const std::size_t size  = static_cast<std::size_t>(state.range(0)) ;
    const std::size_t count = (1024*32) / size ;

    alignas(64) static std::byte buffer[1024*32] ;
    std::pmr::monotonic_buffer_resource source(buffer, sizeof(buffer), std::pmr::null_memory_resource()) ;

    for(auto _ : state){
        for(std::size_t i = {0uz} ; i < count ; ++i){
            auto ptr = source.allocate(size, 1) ;
            benchmark::DoNotOptimize(ptr) ;
        }
        benchmark::ClobberMemory() ;

        // Reset the internal bump pointer to the start of the buffer for the next iteration
        source.release() ;
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count)) ;
}
BENCHMARK(monotonic_buffer_resource_L1D)->RangeMultiplier(4)
                                    ->Range(64 , 16 * 1024)
                                    ->MinTime(1)
                                    ->Repetitions(15)
                                    ->DisplayAggregatesOnly(true);

template <std::size_t Size>
static void tip_allocator(benchmark::State& state){
    struct blob{
        char data[Size] ;
    };

    constexpr std::size_t count = (1024*32) / Size ;

    tip::allocator<tip::default_policy , 1024*32> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    for(auto _ : state){
        for(std::size_t i = {0uz} ; i < count ; ++i){
            auto ptr = allocator.allocate<blob>() ;
            benchmark::DoNotOptimize(ptr) ;
        }
        benchmark::ClobberMemory() ;

        allocator.reclaim_memory() ;
        benchmark::ClobberMemory() ;
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count)) ;
}

static void allocator_default_policy_L1D(benchmark::State& state){
    switch(state.range(0)){
        case 64    : tip_allocator<64>(state)    ; break ;
        case 256   : tip_allocator<256>(state)   ; break ;
        case 1024  : tip_allocator<1024>(state)  ; break ;
        case 4096  : tip_allocator<4096>(state)  ; break ;
        case 16384 : tip_allocator<16384>(state) ; break ;
        default    : state.SkipWithError("unsupported size") ;
    }
}
BENCHMARK(allocator_default_policy_L1D)->RangeMultiplier(4)
                                   ->Range(64 , 16 * 1024)
                                   ->MinTime(1)
                                   ->Repetitions(15)
                                   ->DisplayAggregatesOnly(true);

static void monotonic_buffer_resource_reset_only(benchmark::State& state){
    alignas(64) static std::byte buffer[1024*32] ;
    std::pmr::monotonic_buffer_resource source(buffer, sizeof(buffer), std::pmr::null_memory_resource()) ;

    for(auto _ : state){
        source.release() ;
        benchmark::ClobberMemory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(monotonic_buffer_resource_reset_only)->MinTime(1)
                                               ->Repetitions(15)
                                               ->DisplayAggregatesOnly(true);

static void allocator_reset_only(benchmark::State& state){
    tip::allocator<tip::default_policy , 1024*32> allocator ;

    benchmark::DoNotOptimize(allocator.expose_internal_pool_start()) ;

    for(auto _ : state){
        allocator.reclaim_memory() ;
        benchmark::ClobberMemory() ;
    }

    state.SetItemsProcessed(state.iterations()) ;
}
BENCHMARK(allocator_reset_only)->MinTime(1)
                               ->Repetitions(15)
                               ->DisplayAggregatesOnly(true);
