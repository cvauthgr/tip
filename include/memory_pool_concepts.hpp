#pragma once

#include <concepts>
#include <cstddef>

namespace tip {

template <std::size_t size>
concept isMultipleOfTwo = requires {
    size % 2 == 0 ;
};

template <std::size_t size>
concept isNonZero = requires {
    size != 0 ;
};

template <typename Policy>
concept AllocatorPolicy = requires {
    {Policy::bounds_checks}          -> std::convertible_to<bool> ;
    {Policy::ownership_checks}       -> std::convertible_to<bool> ;
    {Policy::can_fit_checks}         -> std::convertible_to<bool> ;
    {Policy::nullptr_checks}         -> std::convertible_to<bool> ;
    {Policy::size_check_validations} -> std::convertible_to<bool> ;
    {Policy::fallback_resource}      -> std::convertible_to<bool> ;
    {Policy::upstream_resource}      -> std::convertible_to<bool> ;
};

}
