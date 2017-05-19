# pragma once

#include "detail/detail.hpp"

#include <iterator> // std::iterator_traits<...>::value_type
#include <vector>   // std::vector

#include <no_tbb/no_tbb.hpp>

#include <ittnotify.h>


namespace radix_sort {
namespace per_thread {

template<typename value_t>
struct data {
    typedef typename detail::radix_sort_helper<value_t> helper_type;
    data()
        : frequency(helper_type::num_buckets)
    {}
    std::vector<size_t> frequency;
};
}
 
static __itt_domain* the_domain = __itt_domain_createA("concurrent_sort");

template<typename iterator_t>
void concurrent_sort(iterator_t begin, iterator_t end) {

    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;
    typedef typename detail::radix_sort_helper<value_type>::radix_type radix_type;
    
    assert(begin <= end);
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));

    no_tbb::thread_pool& p = no_tbb::thread_pool::instance();
    size_t num_threads = p.num_threads();

    size_t aligned_num_elements = no_tbb::align(num_elements, num_threads);
    size_t num_elements_per_thread = aligned_num_elements / num_threads;

    typedef std::vector<detail::no_init<value_type> > no_init_vector_type;
    no_init_vector_type next_iter_array(num_elements);
    std::vector<size_t>     bucket_sizes(helper_type::num_buckets);

    std::vector<typename no_init_vector_type::iterator > buckets;
    buckets.reserve(helper_type::num_buckets);

    for (size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        std::vector<per_thread::data<value_type> > thread_data(num_threads);

        // calculate per thread frequencies
        no_tbb::parallel_for(0, num_elements, [&thread_data, ii, begin](size_t thread_id, size_t jj) -> void {
            static __itt_string_handle* this_name = __itt_string_handle_createA("mk_freq");
            __itt_task_begin(the_domain, __itt_null, __itt_null, this_name);

            per_thread::data<value_type>& this_thread_data = thread_data[thread_id];
            this_thread_data.frequency[helper_type::digit(ii, begin[jj])]++;

            __itt_task_end(the_domain);
        });
                
        // conver frequencies to write offsets, resize buckets
        no_tbb::parallel_for(0, helper_type::num_buckets, [&thread_data, &bucket_sizes, num_threads](size_t thread_id, size_t jj) -> void {
            static __itt_string_handle* this_name = __itt_string_handle_createA("Freq_to_offset");
            __itt_task_begin(the_domain, __itt_null, __itt_null, this_name);
            size_t current_sum = 0;
            size_t next_sum = 0;
            for(size_t kk = 0; kk < num_threads; ++kk) {
                size_t next_sum = current_sum + thread_data[kk].frequency[jj];
                thread_data[kk].frequency[jj] = current_sum;
                current_sum = next_sum;
            }
            
            bucket_sizes[jj] = current_sum;
            __itt_task_end(the_domain);
        });

        // Map buckets to the next_iter_array
        buckets.push_back(next_iter_array.begin());
        for (size_t jj = 1; jj < helper_type::num_buckets; ++jj) {
            buckets.push_back(buckets.back() + bucket_sizes[jj - 1]);
        }

        // populate buckets
        no_tbb::parallel_for(0, num_elements, [&thread_data, &buckets, ii, begin](size_t thread_id, size_t jj) -> void {
            static __itt_string_handle* this_name = __itt_string_handle_createA("populate_buckets");
            __itt_task_begin(the_domain, __itt_null, __itt_null, this_name);

            radix_type digit = helper_type::digit(ii, begin[jj]); 
            size_t write_offset = thread_data[thread_id].frequency[digit]++;
            buckets[digit][write_offset] = begin[jj]; 

            __itt_task_end(the_domain);
        });

        // dump buckets back to the resulting buffer
        no_tbb::parallel_for(0, num_elements, [begin, &next_iter_array](size_t thread_id, size_t jj) -> void {
            static __itt_string_handle* this_name = __itt_string_handle_createA("copy_back");
            __itt_task_begin(the_domain, __itt_null, __itt_null, this_name);
            begin[jj] = next_iter_array[jj];
            __itt_task_end(the_domain);
        });

        buckets.clear();
    }
}
    
}
