#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <print>
#include <utility>

//#define INFO

#include "memory_pool_concepts.hpp"

template <std::size_t size>
requires isNonZero<size> && isMultipleOfTwo<size>
class memory_pool {
    private :
        void* initial_start_ = nullptr ;
        void* bump_ptr_ = nullptr ;
        std::size_t capacity_ = size ;

        // Implementation for checkpoints
        // Might not like it and remove it later
        //
        // The idea is that you request a checkpoint
        // through an API like .cache_checkpoint()
        // and internally the last known location
        // of the bump pointer is saved . When
        // through another API like revert() we request
        // to move back to a specified checkpoint and
        // continue allocating from that point forward
        // The objects are simply over-written
        //
        // This will obviously need to support
        // allocations from arrays of object

        struct alignas(64) checkpoint_{
            // Bump reset
            void* checkpoint_ptr = nullptr ;
            // Capacity reset
            std::size_t last_known_capacity = 0 ;

            checkpoint_() = delete ;

            explicit checkpoint_(void* current_bump_location , std::size_t current_capacity)
                                 : checkpoint_ptr {current_bump_location} , last_known_capacity {current_capacity}
            {
                #ifdef INFO
                std::println("Checkpoint cached : Current Pointer -> {} | Current capacity -> {}",
                             static_cast<void*>(checkpoint_ptr),
                             last_known_capacity);
                #endif
            }
        };

        checkpoint_ cached_ = checkpoint_(initial_start_ , size) ;

    public :
        explicit memory_pool() {
            // Try to allocate (size) raw bytes
            initial_start_ = ::operator new(size) ;
            bump_ptr_ = initial_start_ ;
            if(initial_start_ != nullptr && bump_ptr_ != nullptr){
                #ifdef INFO
                std::println("Pool allocated");
                #endif
            }
        }

        // Delete the copy constructor and the
        // copy-assignment constructor , the move and move
        // assignment constructors get nuked by the compiler
        memory_pool(const memory_pool &) = delete;
        memory_pool &operator=(const memory_pool &) = delete ;

        ~memory_pool() {
          // Delete the allocated memory
          ::operator delete(initial_start_);
          #ifdef INFO
          std::println("Pool destructed");
          #endif
        }

        [[nodiscard]] inline constexpr auto acquire_internal_pool_start() const noexcept -> const void* {
            return initial_start_ ;
        }

        template <typename Obj>
        [[nodiscard]] auto allocate(std::size_t alignment = alignof(Obj)) -> Obj* {
            // Try to align our bump pointer
            #ifdef INFO
            std::println("Arguments : {} , {} , {} , {}",alignment , sizeof(Obj) , bump_ptr_ , capacity_);
            #endif
            auto aligned_ptr = std::align(alignment , sizeof(Obj) , bump_ptr_ , capacity_) ;

            if(aligned_ptr != nullptr){
                #ifdef INFO
                std::println("aligned_ptr is not nullptr -> {}",aligned_ptr) ;
                #endif
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
            #ifdef INFO
            std::println("aligned_ptr was nullptr");
            #endif
            return nullptr ;
        }

        // Could be named just claim_and_construct
        // Also ignore any safety checks because
        // this is C++ duh
        template <typename Obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct(Args&&... args) -> Obj* {
            auto location = this->allocate<Obj>() ;
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

            auto location = this->allocate_array<Obj>(count) ;
            #ifdef INFO
            std::println("First element aligned pointer : {}",static_cast<void*>(location)) ;
            #endif
            if(!static_cast<bool>(location)){
                #ifdef INFO
                std::println("Location is nullptr") ;
                #endif
                return nullptr ;
            }
            // Already constructed one , construct count-1 objects
            Obj* first_constructed = std::construct_at(location , std::forward<Args>(args)...) ;
            #ifdef INFO
            std::println("Location of first constructed object : {}",static_cast<void*>(first_constructed));
            #endif
            // The location pointer if not nullptr
            // will already be aligned to Obj
            //
            // I really don't like this loop
            for(std::size_t i = {1uz} ; i < count ; ++i){
                std::construct_at(location + i, std::forward<Args>(args)...);
                #ifdef INFO
                std::println("Next object constructed at : {} | Index : {}",static_cast<void*>(next) , i) ;
                #endif
            }
            #ifdef INFO
            std::println("Returned first_constructed : {}",static_cast<void*>(first_constructed)) ;
            #endif
            return first_constructed ;
        }

        constexpr inline auto remaining_capacity() const noexcept -> std::size_t {
            return capacity_ ;
        }

        constexpr auto reclaim() noexcept -> void {
            // Just return everything to default
            // Easy and simple RAHHHHH
            #ifdef INFO
            std::println("Pool reclaimed");
            #endif

            bump_ptr_ = initial_start_ ;
            capacity_ = size ;

            return ;
        }

        constexpr auto owns(const void* ptr) const noexcept -> bool {
            if(ptr >= initial_start_ && ptr <= static_cast<std::byte*>(initial_start_)+size){
                return true ;
            }

            return false ;
        }

        constexpr auto cache_checkpoint() -> void {
            // Cache the checkpoint
            cached_ = checkpoint_(bump_ptr_ , capacity_) ;

            return ;
        }

        // Not a general fuck around untill you find out API
        // Can ONLY move you back one time !!!
        constexpr auto revert() -> void {
            // An one time undo (from the point of caching the checkpoint)
            #ifdef INFO
            std::println("Reverted to last cached checkpoint");
            #endif
            bump_ptr_ = cached_.checkpoint_ptr ;
            capacity_ = cached_.last_known_capacity ;
            #ifdef INFO
            std::println("Current location of bump pointer -> {}\nCurrent capacity -> {}",bump_ptr_,capacity_);
            #endif

            return ;
        }

        constexpr inline auto used_memory() const noexcept -> std::size_t {
            return size - capacity_ ;
        }

        // Make it return a span RAHHHHHHH (too slow for my liking)
        // AND NO SAFETY CHECKING BECAUSE IT IS C++
        template <typename Obj>
        auto allocate_array(std::size_t count) -> Obj* {
            // Allocate the first object
            // The pointer returned is pointing at the start
            // of the first object "allocated"
            Obj* first_in_line = this->allocate<Obj>() ;
            // Check if for count we stay inside the capacity
            //
            // We need to check if the last object is fully inside
            // the memory pool
            Obj* ptr_to_last = first_in_line + (count + 1);
            // If we stay inside return it
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
            #ifdef INFO
            std::println("Count is invalid . Not enough capacity . Only one object or none \"allocated\"") ;
            #endif
            return nullptr ;
        }

        // Get bump allocator position , will be used
        // for testing the checkpoint feature
        constexpr inline auto acquire_bump() const noexcept -> const void* {
            return bump_ptr_ ;
        }

};
