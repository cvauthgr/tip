#pragma once

#include <cstddef>
#include <new>

#include "memory_pool_concepts.hpp"

namespace tip {

template <tip::AllocatorPolicy Policy , std::size_t Size>
requires tip::isMultipleOfTwo<Size> && tip::isNonZero<Size>
class fallback_resource {
    private :
        void* beginning_of_resource_ = nullptr ;

    public :
       explicit fallback_resource(){
           if constexpr (Policy::use_reserve){
               beginning_of_resource_ = ::operator new(Size) ;
           }
       }

       ~fallback_resource(){
           if constexpr (Policy::use_reserve){
               ::operator delete(beginning_of_resource_) ;
           }
       }

       // Expose the basic methods for acquiring this pool

       [[nodiscard]] inline auto acquire_fallback_resource_start() const noexcept -> void* {
           return beginning_of_resource_ ;
       }

       [[nodiscard]] inline auto acquire_fallback_resource_size() const noexcept -> std::size_t {
           return Size ;
       }
};

}// End of tip namespace
