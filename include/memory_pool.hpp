#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>

#include "memory_pool_concepts.hpp"
#include "policies.hpp"
#include "unsafe_align.hpp"

namespace tip {

template <AllocatorPolicy Policy = tip::default_policy,
          std::size_t Size = 2048>
requires isMultipleOfTwo<Size>
class memory_pool {
    private :
        void* initial_start_ = nullptr ;
        void* bump_ptr_ = nullptr ;
        std::size_t capacity_ = Size ;

        public :
        explicit memory_pool() {
            // Try to allocate (size) raw bytes
            // only if the size in non-zero
            if constexpr (Size != 0) {
                initial_start_ = ::operator new(Size) ;
                bump_ptr_ = initial_start_ ;
            }
        }

        // Delete the copy constructor and the
        // copy-assignment constructor , the move and move
        // assignment constructors get nuked by the compiler
        memory_pool(const memory_pool& pool) = delete;
        memory_pool& operator=(const memory_pool& pool) = delete ;

        // A move constructor would be useful for the fallback_resource
        memory_pool& operator=(void* ptr_to_fallback){
            bump_ptr_ = ptr_to_fallback ;
            return *this ;
        }

        ~memory_pool() {
          // Delete the allocated memory
          // only if we allocated
          if constexpr (Size != 0) {
              ::operator delete(initial_start_);
          }
        }

        [[nodiscard]] inline constexpr auto acquire_internal_pool_start() const noexcept -> void* {
            return initial_start_ ;
        }

        template <typename Obj>
        [[nodiscard]] auto claim(std::size_t alignment = alignof(Obj)) -> Obj* {
            void* aligned_ptr = nullptr ;
            // Try to align our bump pointer
            if constexpr(!Policy::unsafe_align_ptrs){
                aligned_ptr = std::align(alignment , sizeof(Obj) , bump_ptr_ , capacity_) ;
            }

            if constexpr(Policy::unsafe_align_ptrs){
                aligned_ptr = unsafe_align(alignment , bump_ptr_ , capacity_) ;
            }

            if constexpr(Policy::nullptr_checks){
                if(aligned_ptr != nullptr){
                // Bytes used for alignment have already been subtracted from capacity_
                // Now subtract the sizeof(obj)
                capacity_ -= sizeof(Obj) ;
                // The bump_ptr_ has been moved by the space needed for alignment not
                // the sizeof(obj)
                bump_ptr_ = static_cast<Obj*>(bump_ptr_) + 1 ;
                // std::align returns a void* so we have to cast it to obj*
                return static_cast<Obj*>(aligned_ptr) ;
            }

                // If std::align fails for some reason (not enough space etc.)
                // return a nullptr , peak problem solving (~67ULLIQ)
                return nullptr ;
            }

            capacity_ -= sizeof(Obj) ;
            bump_ptr_ = static_cast<Obj*>(bump_ptr_) + 1 ;

            return static_cast<Obj*>(aligned_ptr) ;
        }

        constexpr inline auto remaining_capacity() const noexcept -> std::size_t {
            return capacity_ ;
        }

        constexpr auto reclaim() noexcept -> void {
            // Just return everything to default
            // Easy and simple RAHHHHH

            bump_ptr_ = initial_start_ ;
            capacity_ = Size ;

            return ;
        }

        constexpr auto owns(const void* ptr) const noexcept -> bool {
            auto pool_start = reinterpret_cast<uintptr_t>(initial_start_) ;
            auto pool_end = pool_start + Size ;
            auto ptr_to_be_checked = reinterpret_cast<uintptr_t>(ptr) ;

            return (ptr_to_be_checked >= pool_start) && (ptr_to_be_checked <= pool_end) ;
        }


        constexpr inline auto used_memory() const noexcept -> std::size_t {
            return Size - capacity_ ;
        }

        // Make it return a span RAHHHHHHH (too slow for my liking)
        // AND NO SAFETY CHECKING BECAUSE IT IS C++
        template <typename Obj>
        [[nodiscard]] auto claim_array(std::size_t count) -> Obj* {

            if constexpr (Policy::can_fit_checks) {
                // Here I will implement the fix can_fit() was created for
                if(!this->can_fit<Obj>(count)){
                    return nullptr ;
                }
            }
            // Allocate the first object
            // The pointer returned is pointing at the start
            // of the first object "allocated"
            Obj* first_in_line = this->claim<Obj>() ;
            // Check if for count we stay inside the capacity
            //
            // We need to check if the last object is fully inside
            // the memory pool
            Obj* ptr_to_last = first_in_line + count;
            // If we stay inside return it
            if constexpr (Policy::ownership_checks) {
                if(this->owns(ptr_to_last)){
                    // We need to also handle capacity
                    // decrement if the "allocation" is
                    // valid
                    //
                    // As the bump already got aligned and
                    // the capacity of 1 object got decremented
                    // we only need to subtract the sizeof(obj)*(count-1)
                    capacity_ -= sizeof(Obj)*(count-1) ;

                    // Lastly we also need to move the bump pointer
                    // which is already aligned by std::align
                    bump_ptr_ = static_cast<Obj*>(bump_ptr_)+(count-1) ;
                    return first_in_line ;
                }

                // If the pointer is outside , prompt (for now)
                // then return nullptr
                return nullptr ;
            }
            capacity_ -= sizeof(Obj)*(count-1) ;
            bump_ptr_ = static_cast<Obj*>(bump_ptr_)+(count-1) ;
            return first_in_line ;
        }

        // Get bump allocator position , will be used
        // for testing the checkpoint feature
        constexpr inline auto acquire_bump() const noexcept -> const void* {
            return bump_ptr_ ;
        }

        // Checks if an array allocation is possible
        // More in problems.md
        template <typename Obj>
        [[nodiscard]] inline constexpr auto can_fit(std::size_t count = 1) const noexcept -> bool {
            // Make an unavoidable copy of the bump ptr because the call to std::align
            // per our circumstances is GUARANTEED to move it
            auto bump_copy = bump_ptr_ ;
            // Create a virtual buffer so std::align can play without messing with the pool's
            // internal state
            std::size_t virtual_buffer = static_cast<std::size_t>(-1) ;
            void* aligned_ptr = nullptr ;

            if constexpr(!Policy::unsafe_align_ptrs){
                aligned_ptr = std::align(alignof(Obj) , sizeof(Obj)*count , bump_copy , virtual_buffer) ;
            }

            if constexpr(Policy::unsafe_align_ptrs){
                aligned_ptr = unsafe_align(alignof(Obj) ,bump_copy , virtual_buffer) ;
            }

            if(this->owns(aligned_ptr) && this->owns(static_cast<Obj*>(aligned_ptr)+count)){
                return true  ;
            }

            return false ;
        }

};

}// End of the tip namespace
