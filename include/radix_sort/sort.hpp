#pragma once

#include "detail/detail.hpp"

#include <iterator> // std::iterator_traits<...>::value_type
#include <vector>   // std::vector

#include <cassert>  

namespace radix_sort {
template<typename iterator_t>
void sort(iterator_t begin, iterator_t end) {
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;

    assert(begin <= end);
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));
    std::vector<value_type> next_iter_array(num_elements);

    for (size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        std::vector<size_t> frequency(helper_type::num_buckets);

        for (size_t jj = 0; jj != num_elements; ++jj) {
            frequency[helper_type::digit(ii, begin[jj])]++;
        }

        size_t count = 0;
        for (size_t jj = 0; jj < helper_type::num_buckets; ++jj) {
            size_t prev_freq = frequency[jj];
            frequency[jj] = count;
            count += prev_freq;
        }

        for (size_t jj = 0; jj != num_elements; ++jj) {
            auto d = helper_type::digit(ii, begin[jj]);
            next_iter_array[frequency[d]] = begin[jj];
            frequency[d]++;
        }

        std::copy(next_iter_array.begin(), next_iter_array.end(), begin);
    }
}
}

