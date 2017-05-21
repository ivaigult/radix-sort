#pragma once

#if defined(TBB_FOUND)

#include <iterator> // std::iterator_traits<...>::value_type
#include <vector>   // std::vector

#include <cassert>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_arena.h>

namespace radix_sort {

template<typename iterator_t>
void tbb_concurrent_sort(iterator_t begin, iterator_t end) {
    tbb::task_arena task_arena;
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;
    typedef typename detail::radix_sort_helper<value_type>::radix_type radix_type;

    assert(begin <= end);
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));

    size_t num_threads = static_cast<size_t>(tbb::this_task_arena::max_concurrency());

    size_t aligned_num_elements = (num_elements +  num_threads - 1) / num_threads * num_threads;
    size_t num_elements_per_thread = aligned_num_elements / num_threads;

    typedef std::vector<detail::no_init<value_type> > no_init_vector_type;
    no_init_vector_type next_iter_array(num_elements);
    std::vector<size_t>     bucket_sizes(helper_type::num_buckets);

    std::vector<typename no_init_vector_type::iterator > buckets;
    buckets.reserve(helper_type::num_buckets);
    typedef std::vector<size_t> frequency_vec_t;

    for (size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        std::vector<std::vector<size_t> > thread_data(num_threads, frequency_vec_t(helper_type::num_buckets));

        // calculate per thread frequencies
        tbb::parallel_for(tbb::blocked_range<size_t>(0, num_elements), [&thread_data, ii, begin](const tbb::blocked_range<size_t>& rr) -> void {
            for (size_t jj = rr.begin(); jj != rr.end(); ++jj) {
                frequency_vec_t& this_thread_data = thread_data[(size_t) tbb::this_task_arena::current_thread_index()];
                this_thread_data[helper_type::digit(ii, begin[jj])]++;
            }
        }, tbb::static_partitioner{});

        // conver frequencies to write offsets, resize buckets
        tbb::parallel_for(tbb::blocked_range<size_t>(0, helper_type::num_buckets), [&thread_data, &bucket_sizes, num_threads](const tbb::blocked_range<size_t>& rr) -> void {
            for (size_t jj = rr.begin(); jj != rr.end(); ++jj) {
                size_t current_sum = 0;
                size_t next_sum = 0;
                for (size_t kk = 0; kk < num_threads; ++kk) {
                    size_t next_sum = current_sum + thread_data[kk][jj];
                    thread_data[kk][jj] = current_sum;
                    current_sum = next_sum;
                }
                bucket_sizes[jj] = current_sum;
            }
        }, tbb::static_partitioner{});

        // Map buckets to the next_iter_array
        buckets.push_back(next_iter_array.begin());
        for (size_t jj = 1; jj < helper_type::num_buckets; ++jj) {
            buckets.push_back(buckets.back() + bucket_sizes[jj - 1]);
        }

        // populate buckets
        tbb::parallel_for(tbb::blocked_range<size_t>(0, num_elements), [&thread_data, &buckets, ii, begin](const tbb::blocked_range<size_t>& rr) -> void {
            for (size_t jj = rr.begin(); jj != rr.end(); ++jj) {
                radix_type digit = helper_type::digit(ii, begin[jj]);
                size_t write_offset = thread_data[(size_t)tbb::this_task_arena::current_thread_index()][digit]++;
                buckets[digit][write_offset] = begin[jj];
            }
        }, tbb::static_partitioner{});

        // dump buckets back to the resulting buffer
        tbb::parallel_for(tbb::blocked_range<size_t>(0, num_elements), [begin, &next_iter_array](const tbb::blocked_range<size_t>& rr) -> void {
            for (size_t jj = rr.begin(); jj != rr.end(); ++jj) {
                begin[jj] = next_iter_array[jj];
            }
        }, tbb::static_partitioner{});

        buckets.clear();
    }
}
    
}

#endif