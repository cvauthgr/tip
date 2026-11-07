#pragma once

#include <cstddef>
#include <cstdint>

namespace tip{

inline auto unsafe_align(std::size_t alignment,
                                   void*& ptr_to_align,
                                   std::size_t& capacity)
noexcept -> void* {
  const auto to_be_aligned = reinterpret_cast<uintptr_t>(ptr_to_align);
  const auto aligned_ptr = (to_be_aligned - 1u + alignment) & -alignment;
  capacity -=  aligned_ptr - to_be_aligned ;
  return ptr_to_align = reinterpret_cast<void*>(aligned_ptr);
}

}// End of tip namespace
