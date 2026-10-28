#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <print>

#include "memory_pool_concepts.hpp"

template <std::size_t size>
requires isMultipleOfTwo<size>
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

            explicit checkpoint_(void*& current_bump_location , std::size_t current_capacity)
                                 : checkpoint_ptr {current_bump_location} , last_known_capacity {current_capacity}
            {
                std::println("Checkpoint cached : Current Pointer -> {} | Current capacity -> {}",
                             static_cast<void*>(checkpoint_ptr),
                             last_known_capacity);
            }
        };

        checkpoint_ cached_ = { } ;

    public :
        explicit memory_pool() {
            // Try to allocate (size) raw bytes
            initial_start_ = ::operator new(size) ;
            bump_ptr_ = initial_start_ ;
            if(initial_start_ != nullptr && bump_ptr_ != nullptr){
                std::println("Pool allocated");
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

          std::println("Pool destructed");
        }

        template <typename obj>
        [[nodiscard]] auto allocate(std::size_t alignment = alignof(obj)) -> obj* {
            // Try to align our bump pointer
            std::println("Arguments : {} , {} , {} , {}",alignment , sizeof(obj) , bump_ptr_ , capacity_);
            auto aligned_ptr = std::align(alignment , sizeof(obj) , bump_ptr_ , capacity_) ;

            if(aligned_ptr != nullptr){
                std::println("aligned_ptr is not nullptr -> {}",aligned_ptr) ;
                // Bytes used for alignment have already been subtracted from capacity_
                // Now subtract the sizeof(obj)
                capacity_ -= sizeof(obj) ;
                // The bump_ptr_ has been moved by the space needed for alignment not
                // the sizeof(obj)
                bump_ptr_ = static_cast<obj*>(bump_ptr_) + 1 ;
                // std::align returns a void* so we have to cast it to obj*
                return static_cast<obj*>(aligned_ptr) ;
            }

            // If std::align fails for some reason (not enough space etc.)
            // return a nullptr , peak problem solving (~67ULLIQ)
            std::println("aligned_ptr was nullptr");
            return nullptr ;
        }

        // Could be named just create_instance
        template <typename obj , typename... Args>
        [[nodiscard]] auto allocate_and_construct(Args&&... args) -> obj* {
            auto location = this->allocate<obj>() ;
            return std::construct_at(location , std::forward<Args>(args)...) ;
        }

        constexpr auto remaining_capacity() const noexcept -> std::size_t {
            return capacity_ ;
        }

        constexpr auto reclaim() noexcept -> void {
            // Just return everything to default
            // Easy and simple RAHHHHH
            bump_ptr_ = initial_start_ ;
            capacity_ = size ;
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
        }

        // Not a general fuck around untill you find out API
        // Can ONLY move you back one time !!!
        constexpr auto revert() -> void {
            // An one time undo (from the point of caching the checkpoint)
            bump_ptr_ = cached_.checkpoint_ptr ;
            capacity_ = cached_.last_known_capacity ;
        }

        constexpr auto used_memory() const noexcept -> std::size_t {
            return size - capacity_ ;
        }

        /*template <typename... Objects , typename... Args>
            auto allocate_range() -> */
};
