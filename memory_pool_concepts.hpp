#pragma once

#include <cstddef>

template <std::size_t size>
concept isMultipleOfTwo = requires {
    size % 2 == 0;
};
