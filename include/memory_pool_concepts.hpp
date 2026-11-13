#pragma once

#include <concepts>
#include <cstddef>

namespace tip {

template <std::size_t Size>
concept isMultipleOfTwo = requires {
    Size % 2 == 0 ;
};

// Useless for now
template <std::size_t Size>
concept isNonZero = requires {
    Size != 0 ;
};

template <typename Policy>
concept AllocatorPolicy = requires {
    {Policy::ownership_checks}       -> std::convertible_to<bool> ;
    {Policy::can_fit_checks}         -> std::convertible_to<bool> ;
    {Policy::nullptr_checks}         -> std::convertible_to<bool> ;
    {Policy::unsafe_align_ptrs}      -> std::convertible_to<bool> ;
    {Policy::size_check_validations} -> std::convertible_to<bool> ;
    {Policy::fallback_to_new}        -> std::convertible_to<bool> ;
    {Policy::enable_usage_tracking}  -> std::convertible_to<bool> ;
    {Policy::pool_chaining}          -> std::convertible_to<bool> ;
};

}
