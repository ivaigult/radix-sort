#pragma once

#include <iterator>
#include <limits>
#include <cstdint>
#include <array>
#include <deque>

namespace detail {

template<typename value_t, typename radix_t = uint8_t>
struct radix_sort_helper {
    typedef value_t value_type;
    typedef radix_t radix_type;
    enum { num_digits = sizeof(value_type) / sizeof(radix_type) };
    enum { bits_per_digit = sizeof(radix_type) * 8 };
    
    typedef std::deque<value_t> bucket_type;
    typedef std::array<bucket_type, std::numeric_limits<radix_type>::max()> array_type;

    static radix_type digit(size_t num, value_type value) {
        const radix_type mask = std::numeric_limits<radix_type>::max();
        const size_t bit_shift = bits_per_digit * num;
        return mask & (value >> bit_shift);
    }
};

}

template<typename iterator_t>
void radix_sort(iterator_t begin, iterator_t end) {
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;
    
    typename helper_type::array_type array;
    
    for(size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        for(iterator_t it = begin; it != end; ++it) {
            array[helper_type::digit(ii, *it)].push_back(*it);
        }

        iterator_t it = begin;
        for(typename helper_type::bucket_type& bucket : array) {
            for(const value_type& value: bucket) {
                *(it++) = value;
            }
            bucket.clear();
        }
    }
}

