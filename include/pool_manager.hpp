#pragma once

// The pool manager only handles memory pools
// It makes sure that when the allocator signals
// that the current pool has run out of memory
// another pool will be allocated and the necessary
// steps will be taken . Those steps are :
//
// -> The bump pointer is placed at the start of the new pool
// -> The capacity is now reset to the new size of the new pool
//
// -> IMPORTANT : THE ALLOCATOR NOW TRACKS THE INTERNAL POOL START TOO , THIS NEEDS TO BE
// ACCOUNTED FOR TOO
//
// Probably shouldn't be mentioned here but fallback_to_new + pool_chaining should be prohibited

#include <array>
#include <cstddef>

#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"

namespace tip {

template <AllocatorPolicy Policy,
          std::size_t Size,
          std::size_t MaxPoolsChained = 1>
class pool_manager {
private:
  std::array<memory_pool<Policy, Size> *, MaxPoolsChained> pools_ = {};
  memory_pool<Policy, Size> *pool_resource_ = nullptr;
  std::size_t pool_index_ = 0;

  // Notifier that the pool manager won't keep
  // chaining memory pools while MaxPoolChained is reached
  bool stopped_chaining_ = false;

public:
  pool_manager() = default ;

  // Non-copyable and non-movable


  // On construction set the first element
  // of the array to the start of the first pool
  auto inline constexpr operator()(tip::memory_pool<Policy,Size>& pool) -> void {
    if constexpr (MaxPoolsChained != 0 || !Policy::pool_chaining) {
          // This is bad . Mark my words
          pools_[pool_index_] = &pool;
          pool_resource_ = &pool;

          pool_index_++;
    }
  }

  // Default destructor
  // The pools clean up after
  // themselves but we have to
  // clean the pointers to the
  // pools themselves
  //
  // Delete all the dynamically
  // allocated ones not the one
  // created statically
  ~pool_manager() {
    if constexpr (MaxPoolsChained != 0 || !Policy::pool_chaining) {
      for (std::size_t delete_index = {1uz}; delete_index < pools_.size(); ++delete_index) {
        delete pools_[delete_index] ;
      }
    }
  }

  // One API creating the next pool
  // and making the necessary changes
  // I stated
  auto inline constexpr expand_pool_chain() noexcept -> void {
    if constexpr (MaxPoolsChained != 0 || !Policy::pool_chaining) {
      if (stopped_chaining_) [[unlikely]] {
        return;
      }

      if (pools_.size() == Size) [[unlikely]] {
        stopped_chaining_ = true;
        return;
      }

      // Allocate the next pool
      auto new_pool = new memory_pool<Policy, Size>;
      // And save it
      pools_[pool_index_] = new_pool;

      // Make the needed resets
      // to the main pool we are handling
      pool_resource_->reseat_bump_pointer(new_pool->acquire_internal_pool_start());

      // Increment the pool index
      pool_index_++;

      return;
    }

    return;
  }
};

}// End of the tip namespace
