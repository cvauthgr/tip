#pragma once

#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"
#include "pool_manager.hpp"

#include <cstddef>
#include <cstdint>

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

    public:

        allocator(){
            // If fallback to new and pool chaining are
            // enabled explode , quite literally
            if constexpr (Policy::fallback_to_new && Policy::pool_chaining) {
                __builtin_trap() ;
            }
            manager_(pool_) ;
        }

        ~allocator() = default ;

        // Non-copyable and non-movable
        allocator(const allocator& allocator) = delete ;
        allocator& operator=(const allocator& allocator) = delete ;

        template <typename Obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct(Args&&... args) -> Obj* {
            if constexpr (Policy::fallback_to_new && Policy::size_check_validations) {
                if(switched_to_fallback_){
                    return new Obj(std::forward<Args>(args)...) ;
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
                    return new Obj(std::forward<Args>(args)...) ;
                }
            }

            // Just before allocating we check if there is enough
            // space for the object . If there is not add another pool

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
            if constexpr (Policy::fallback_to_new && Policy::size_check_validations) {
                if(switched_to_fallback_){
                    return new Obj[count]{{args...}} ;
                }
                auto bump_location = pool_.acquire_bump() ;

                std::size_t worst_scenario_offset = reinterpret_cast<uintptr_t>(bump_location) ;
                worst_scenario_offset += ((alignof(Obj)-1)+(count*sizeof(Obj))) ;

                if(!pool_.owns(reinterpret_cast<const void*>(worst_scenario_offset))){
                    switched_to_fallback_ = true ;
                    return new Obj[count]{{args...}} ;
                }
            }

            // Just before allocating we check if there is enough
            // space for the object . If there is not add another pool

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
            return switched_to_fallback_ ;
        }

        constexpr inline auto remaining_capacity() const noexcept -> std::size_t {
            return pool_.remaining_capacity() ;
        }

        constexpr inline auto reclaim_memory() noexcept -> void {
            pool_.reclaim() ;
            return ;
        }
};

}// End of tip namespace
