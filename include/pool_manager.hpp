#pragma once

// The pool manager only handles memory pools
// It makes sure that when the allocator signals
// that the current pool has run out of memory
// another pool will be allocated and the necessary
// steps will be taken . Those steps are :
//
// -> The bump pointer is placed at the start of the new pool
// -> The capacity is now reset to the new size of the new pool
// -> Any cached checkpoints are invalidated
//
// Probably shouldn't be mentioned here but fallback_to_new + pool_chaining should be prohibited

#include <array>
#include <cstddef>

#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"

#define SINGLE_POOL 1
#define NO_POOLS 0

namespace tip {

template <AllocatorPolicy Policy,
          std::size_t Size,
          std::size_t MaxPoolsChained>
          requires isNonZero<MaxPoolsChained> &&
                   (MaxPoolsChained > SINGLE_POOL || MaxPoolsChained == NO_POOLS)
class pool_manager {
private:
  std::array<memory_pool<Policy, Size> *, MaxPoolsChained> pools_ = {};
  memory_pool<Policy, Size> *pool_resource_ = nullptr;
  std::size_t pool_index_ = 0;

  // Notifier that the pool manager won't keep
  // chaining memory pools while MaxPoolChained is reached
  bool stopped_chaining_ = false;

public:
  // On construction set the first element
  // of the array to the start of the first pool
  explicit pool_manager(memory_pool<Policy, Size> pool) {
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
      for (std::size_t delete_index = {1uz}; delete_index < pools_.size();
           ++delete_index) {
        delete static_cast<const std::byte *>(pools_[delete_index]);
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
      pool_resource_->reseat_bump_pointer(
          new_pool->acquire_internal_pool_start());
      pool_resource_->invalidate_cached_checkpoint(
          new_pool->acquire_internal_pool_start());

      // Increment the pool index
      pool_index_++;

      return;
    }

    return;
  }
};

}// End of the tip namespace
