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

#include <cassert>
#include <vector>
#include <cstddef>

#include "memory_pool.hpp"
#include "memory_pool_concepts.hpp"

#define MAX_CHAIN_BEFORE_ALLOC 16

namespace tip {

template <AllocatorPolicy Policy,
          std::size_t Size
          >
class pool_manager {
private:
  std::vector<memory_pool<Policy, Size>*> pools_ = {};
  memory_pool<Policy, Size>* pool_resource_ = nullptr;

public:
  pool_manager() = default ;

  // Non-copyable and non-movable
  pool_manager(const pool_manager& pool) = delete;
  pool_manager& operator=(const pool_manager& pool) = delete ;

  // On construction set the first element
  // of the array to the start of the first pool
  auto inline constexpr operator()(tip::memory_pool<Policy,Size>& pool) -> void {
    pools_.reserve(MAX_CHAIN_BEFORE_ALLOC) ;
    if constexpr (Policy::pool_chaining) {
          // This is bad . Mark my words
          pool_resource_ = &pool;
    }
  }

  // The pools clean up after
  // themselves , we only need
  // to clean the pointers we
  // got from new
  ~pool_manager(){
      for(auto* ptr : pools_){
              delete ptr ;
      }
  }

  // One API creating the next pool
  // and making the necessary changes
  // I stated
  auto inline constexpr expand_pool_chain() noexcept -> void {
    if constexpr (Policy::pool_chaining) {
      // Allocate the next pool
      memory_pool<Policy,Size>* new_pool = new memory_pool<Policy, Size>(); // <- Leak exactly here
      // And save it
      pools_.push_back(new_pool);
      pool_resource_ = new_pool ;
      return;
    }

    return;
  }

  [[nodiscard]] auto inline constexpr get_active_pool() const noexcept -> tip::memory_pool<Policy,Size>* {
      return pool_resource_ ;
  }

};

}// End of the tip namespace
