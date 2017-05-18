#pragma once

#include <limits>
#include <cstdlib>
#include <cstdint>

namespace radix_sort {
namespace detail {
template<typename value_t, typename radix_t = uint8_t>
struct radix_sort_helper {
    typedef value_t value_type;
    typedef radix_t radix_type;
    static constexpr size_t     num_digits = sizeof(value_type) / sizeof(radix_type);
    static constexpr size_t     bits_per_digit = sizeof(radix_type) * 8;
    static constexpr radix_type max_digit = std::numeric_limits<radix_type>::max();
    static constexpr size_t     num_buckets = max_digit + 1;

    static radix_type digit(size_t num, value_type value) {
        const size_t bit_shift = bits_per_digit * num;
        return max_digit & (value >> bit_shift);
    }
};
}
}