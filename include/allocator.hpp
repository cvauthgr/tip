#pragma once

#include "fallback_resource.hpp"
#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"

#include <cstddef>
#include <cstdint>

namespace tip {

template <AllocatorPolicy Policy , std::size_t N>
requires isNonZero<N>
class allocator{
    private :
        // Manager of the memory_pool
        tip::memory_pool<Policy,N> pool_ ;
        // The fallback resource
        tip::fallback_resource<Policy, N> fallback_resource_ ;

        // The reason for the creation of
        // this class is the handling of
        // different memory pools . We can
        // this way create fallback memory
        // resources and manage more than one
        // pools

        bool in_reserve_ = false ;

    public:

        allocator() = default ;
        ~allocator() = default ;

        // Non-copyable and non-movable
        allocator(const allocator& allocator) = delete ;
        allocator& operator=(const allocator& allocator) = delete ;

        template <typename Obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct(Args&&... args) -> Obj* {
            if constexpr (Policy::use_reserve && Policy::size_check_validations) {
                // Check for the worst case scenatio if the object fits
                // If it doesn't switch the ptr to the fallback resource
                // The allocate call will return nullptr if we don't have
                // a reserve to use
                auto bump_location = pool_.acquire_bump() ;

                std::size_t worst_scenario_offset = reinterpret_cast<uintptr_t>(bump_location) ;
                worst_scenario_offset += ((alignof(Obj)-1)+sizeof(Obj)) ;

                if(!pool_.owns(reinterpret_cast<const void*>(worst_scenario_offset))){
                    pool_ = fallback_resource_.acquire_fallback_resource_start() ;
                    in_reserve_ = true ;
                }
            }

            auto location = pool_.template allocate<Obj>() ;

            if constexpr (Policy::nullptr_checks) {
                // A nullptr check will prevent the
                // SEGFAULT when nullptr gets passed
                // in std::construct_at
                if(!static_cast<bool>(location)){
                    return nullptr ;
                }
            }

            return std::construct_at(location , std::forward<Args>(args)...) ;
        }

        template <typename Obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct_array(std::size_t count , Args&&... args) -> Obj* {
            if constexpr (Policy::use_reserve && Policy::size_check_validations) {
                auto bump_location = pool_.acquire_bump() ;

                std::size_t worst_scenario_offset = reinterpret_cast<uintptr_t>(bump_location) ;
                worst_scenario_offset += ((alignof(Obj)-1)+(count*sizeof(Obj))) ;

                if(!pool_.owns(reinterpret_cast<const void*>(worst_scenario_offset))){
                    pool_ = fallback_resource_.acquire_fallback_resource_start() ;
                    in_reserve_ = true ;
                }
            }
            auto location = pool_.template allocate_array<Obj>(count) ;
            if constexpr (Policy::nullptr_checks){
                if(!static_cast<bool>(location)){
                        return nullptr ;
                }
            }
            // Already constructed one , construct count-1 objects
            // Same here , copies don't move the objects
            Obj* first_constructed = std::construct_at(location , args...) ;
            // The location pointer if not nullptr
            // will already be aligned to Obj
            //
            // I really don't like this loop
            for(std::size_t i = {1uz} ; i < count ; ++i){
                // Make copies instead of moving
                std::construct_at(location + i, args...);
            }
            return first_constructed ;
        }

        constexpr inline auto using_fallback_resource() const noexcept -> bool {
            return in_reserve_ ;
        }

        constexpr inline auto remaining_capacity() const noexcept -> std::size_t {
            return pool_.remaining_capacity() ;
        }
};

}// End of tip namespace
