#pragma once

#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"

#include <cstddef>

namespace tip {

template <AllocatorPolicy Policy , std::size_t N>
class allocator{
    private :
        // Manager of the memory_pool
        tip::memory_pool<Policy,N> pool ;

        // The reason for the creation of
        // this class is the handling of
        // different memory pools . We can
        // this way create fallback memory
        // resources and manage more than one
        // pools

    public:

        template <typename Obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct(Args&&... args) -> Obj* {
            auto location = pool.template allocate<Obj>() ;
            // A nullptr check will prevent the
            // SEGFAULT when nullptr gets passed
            // in std::construct_at
            if(!static_cast<bool>(location)){
                return nullptr ;
            }

            return std::construct_at(location , std::forward<Args>(args)...) ;
        }

        template <typename Obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct_array(std::size_t count , Args&&... args) -> Obj* {

            auto location = pool.template allocate_array<Obj>(count) ;
            if(!static_cast<bool>(location)){
                    return nullptr ;
            }
            // Already constructed one , construct count-1 objects
            Obj* first_constructed = std::construct_at(location , std::forward<Args>(args)...) ;
            // The location pointer if not nullptr
            // will already be aligned to Obj
            //
            // I really don't like this loop
            for(std::size_t i = {1uz} ; i < count ; ++i){
                std::construct_at(location + i, std::forward<Args>(args)...);
            }
            return first_constructed ;
        }

};

}// End of tip namespace
