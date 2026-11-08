#pragma once

#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"
#include "pool_manager.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace tip {

template <AllocatorPolicy Policy,
          std::size_t Size,
          std::size_t MaxPoolsChained = 1>
requires isNonZero<Size>
class allocator{
    private :
        // Manager of the memory_pool
        tip::memory_pool<Policy,Size> pool_ ;
        // The reason for the creation of
        // this class is the handling of
        // different memory pools . We can
        // this way create fallback memory
        // resources and manage more than one
        // pools

        bool switched_to_fallback_ = false ;

        // We will need a way to save the last function
        // call in case we need to chain the pool .
        // Chain the pool and call the expand() API
        // and then return to the last function called
        tip::pool_manager<Policy,Size,MaxPoolsChained> manager_ ;

        // Avoid the function calls to pool and receive it once here
        void* handled_pools_start_ = nullptr ;

    public:

        allocator(){
            // If fallback to new and pool chaining are
            // enabled explode , quite literally
            if constexpr (Policy::fallback_to_new && Policy::pool_chaining) {
                __builtin_trap() ;
            }
            manager_(pool_) ;

            // Acquire the internal pool start
            handled_pools_start_ = pool_.acquire_internal_pool_start() ;
        }

        ~allocator() = default ;

        // Non-copyable and non-movable
        allocator(const allocator& allocator) = delete ;
        allocator& operator=(const allocator& allocator) = delete ;

        template <typename Obj>
        requires std::is_implicit_lifetime_v<Obj> && std::is_default_constructible_v<Obj>
        [[nodiscard]] auto allocate() -> Obj* {
            if constexpr (Policy::fallback_to_new && Policy::size_check_validations) {
                if(switched_to_fallback_){
                    return new Obj() ;
                }
                // Check for the worst case scenario if the object fits
                // If it doesn't switch the ptr to the fallback resource
                // The allocate call will return nullptr if we don't have
                // a reserve to use
                auto bump_location = pool_.acquire_bump() ;

                std::size_t worst_scenario_offset = reinterpret_cast<uintptr_t>(bump_location) ;
                worst_scenario_offset += ((alignof(Obj)-1)+sizeof(Obj)) ;

                if(!pool_.owns(reinterpret_cast<const void*>(worst_scenario_offset))){
                    switched_to_fallback_ = true ;
                    return new Obj() ;
                }
            }

            // Just before allocating we check if there is enough
            // space for the object . If there is not add another pool

            auto location = pool_.template claim<Obj>() ;

            if constexpr (Policy::nullptr_checks) {
                // A nullptr check will prevent the
                // SEGFAULT when nullptr gets passed
                // in std::construct_at
                if(!static_cast<bool>(location)){
                    return nullptr ;
                }
            }

            return std::start_lifetime_as<Obj>(location) ;
        }

        template <typename Obj>
        requires std::is_implicit_lifetime_v<Obj> && std::is_default_constructible_v<Obj>
        [[nodiscard]] auto allocate_array(std::size_t count) -> Obj* {
            if constexpr (Policy::fallback_to_new && Policy::size_check_validations) {
                if(switched_to_fallback_){
                    return new Obj[count]{ } ;
                }
                auto bump_location = pool_.acquire_bump() ;

                std::size_t worst_scenario_offset = reinterpret_cast<uintptr_t>(bump_location) ;
                worst_scenario_offset += ((alignof(Obj)-1)+(count*sizeof(Obj))) ;

                if(!pool_.owns(reinterpret_cast<const void*>(worst_scenario_offset))){
                    switched_to_fallback_ = true ;
                    return new Obj[count]{ } ;
                }
            }

            // Just before allocating we check if there is enough
            // space for the object . If there is not add another pool

            auto location = pool_.template claim_array<Obj>(count) ;

            if constexpr (Policy::nullptr_checks){
                if(!static_cast<bool>(location)){
                        return nullptr ;
                }
            }

            return std::start_lifetime_as_array<Obj>(location , count) ;
        }

        inline auto using_fallback_resource() const noexcept -> bool {
            return switched_to_fallback_ ;
        }

        inline auto remaining_capacity() const noexcept -> std::size_t {
            return pool_.remaining_capacity() ;
        }

        inline auto reclaim_memory() noexcept -> void {
            pool_.reclaim() ;
            return ;
        }

        // For benchmarking use ONLY
        //
        // Exposes the internal pool start pointer
        // Should NEVER be messed with
        inline auto expose_internal_pool_start() const noexcept -> const void* {
            return handled_pools_start_ ;
        }
};

}// End of tip namespace
