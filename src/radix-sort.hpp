#pragma once

#include <limits>   // std::numeric_limits<...>::max()
#include <cstdint>  // std::uint8_t
#include <iterator> // std::iterator_traits<...>::value_type
#include <vector>   // std::vector

#include <cassert>  

#include "no-tbb.hpp"

namespace detail {

template<typename value_t>
struct bucket_node {
    typedef value_t value_type;
    value_t               value;
    bucket_node<value_t>* tail;
};

template<typename value_t>
struct bucket_storage {
    typedef bucket_node<value_t> node_type;

    bucket_storage(size_t num_elements)
        : _num_allocated(0)
        , _data(num_elements)
    {}

    node_type* alloc() { return &_data[_num_allocated++]; }
    void       reset() { _num_allocated = 0; }
private:
    size_t                 _num_allocated;
    std::vector<node_type> _data;
};
    
template<typename value_t>
struct bucket {
    typedef value_t                 value_type;
    typedef bucket_storage<value_t> storage_type;
    typedef bucket_node<value_t>    node_type;

    bucket(storage_type& s)
        : _storage(s)
        , _front(nullptr)
        , _back(nullptr)
        , _size(0)
    {}
    
    void push_back(value_type v) {
        node_type* new_node = _storage.alloc();
        _size++;
        new_node->value = v;
        new_node->tail = nullptr;
        if (!_front) {
            _front = _back = new_node;
            return;
        }

        _back->tail = new_node;
        _back       = new_node;
    }

    void clear() {
        _front = nullptr;
        _back = nullptr;
        _size = 0;
    }

    size_t size() const { return _size; }

    void squash(bucket&& b) {
        if (!b._front)
            return;

        if (!_front) {
            _front = b._front;
            _back  = b._back;
            _size  = b._size;
        } else {
            _back->tail = b._front;
            _back = b._back;
            _size += b._size;
        }
        b.clear();
    }

    struct iterator {
        iterator(node_type* node) : _node(node) {}
        iterator&    operator=(const iterator& it) { _node = it._node; return *this; }
        bool         operator==(const iterator& it) const { return _node == it._node; }
        bool         operator!=(const iterator& it) const { return _node != it._node; }
        iterator&    operator++()    { _node = static_cast<node_type*>(_node->tail); return *this; }
        iterator     operator++(int) {
            node_type* prev_node = _node;
            _node = static_cast<node_type*>(_node->tail);
            return prev_node;
        }
        value_type   operator*() const { return _node->value; }
    private:
        node_type* _node;
    };

    iterator begin() { return _front; }
    iterator end()   { return nullptr; }
    
private:
    storage_type& _storage;
    node_type* _front;
    node_type* _back;
    size_t     _size;
};
    
template<typename value_t, typename radix_t = uint8_t>
struct radix_sort_helper {
    typedef value_t value_type;
    typedef radix_t radix_type;
    static constexpr size_t     num_digits     = sizeof(value_type) / sizeof(radix_type);
    static constexpr size_t     bits_per_digit = sizeof(radix_type) * 8;
    static constexpr radix_type max_digit      = std::numeric_limits<radix_type>::max();
    static constexpr size_t     num_buckets    = max_digit + 1;

    typedef bucket_storage<value_t>   storage_type;
    typedef bucket<value_t>           bucket_type;
    typedef std::vector<bucket_type>  bucket_vec_type;

    static radix_type digit(size_t num, value_type value) {
        const size_t bit_shift = bits_per_digit * num;
        return max_digit & (value >> bit_shift);
    }
};


}

template<typename iterator_t>
void radix_sort(iterator_t begin, iterator_t end) {
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;
    
    assert(begin <= end);
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));

    typename helper_type::storage_type    storage(num_elements);
    typename helper_type::bucket_vec_type array(helper_type::num_buckets, typename helper_type::bucket_type(storage));
    
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
        storage.reset();
    }
}

namespace no_list {
template<typename iterator_t>
void radix_sort(iterator_t begin, iterator_t end) {
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

#include <atomic>

namespace no_list_and_tbb {
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

    
template<typename iterator_t>
void radix_sort(iterator_t begin, iterator_t end) {
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;
    typedef typename detail::radix_sort_helper<value_type>::radix_type radix_type;
    
    assert(begin <= end);
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));

    no_tbb::thread_pool& p = no_tbb::thread_pool::instance();
    size_t num_threads = p.num_threads();

    size_t aligned_num_elements = no_tbb::align(num_elements, num_threads);
    size_t num_elements_per_thread = aligned_num_elements / num_threads;

    std::vector<value_type> next_iter_array(num_elements);
    std::vector<size_t>     bucket_sizes(helper_type::num_buckets);

    std::vector<std::vector<value_type>::iterator > buckets;
    buckets.reserve(helper_type::num_buckets);

    for (size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        std::vector<per_thread::data<value_type> > thread_data(num_threads);

        // calculate per thread frequencies
        no_tbb::parallel_for(0, num_elements, [&thread_data, ii, begin](size_t thread_id, size_t jj) {
            thread_data[thread_id].frequency[helper_type::digit(ii, begin[jj])]++;
        });
                
        // conver frequencies to write offsets, resize buckets
        no_tbb::parallel_for(0, helper_type::num_buckets, [&thread_data, &bucket_sizes, num_threads](size_t thread_id, size_t jj) {
            size_t current_sum = 0;
            size_t next_sum = 0;
            for(size_t kk = 0; kk < num_threads; ++kk) {
                size_t next_sum = current_sum + thread_data[kk].frequency[jj];
                thread_data[kk].frequency[jj] = current_sum;
                current_sum = next_sum;
            }
            bucket_sizes[jj] = current_sum;
        });

        // Map buckets to the next_iter_array
        buckets.push_back(next_iter_array.begin());
        for (size_t jj = 1; jj < helper_type::num_buckets; ++jj) {
            buckets.push_back(buckets.back() + bucket_sizes[jj - 1]);
        }

        // populate buckets
        no_tbb::parallel_for(0, num_elements, [&thread_data, &buckets, ii, begin](size_t thread_id, size_t jj) {
            radix_type digit = helper_type::digit(ii, begin[jj]); 
            size_t write_offset = thread_data[thread_id].frequency[digit]++;
            buckets[digit][write_offset] = begin[jj]; 
        });

        // dump buckets back to the resulting buffer
        no_tbb::parallel_for(0, num_elements, [begin, &next_iter_array](size_t thread_id, size_t jj) -> void {
            begin[jj] = next_iter_array[jj];
        });

        buckets.clear();
    }
}
    
}

namespace no_tbb {
namespace per_thread {

template<typename value_t>
struct buckets {
    buckets(size_t num_elements)
        : storage(num_elements)
        , array(helper_type::num_buckets, typename helper_type::bucket_type(storage))
    {
        int a = 0;
    }

    buckets<value_t>& operator=(const buckets&) = delete;

    typedef detail::radix_sort_helper<value_t> helper_type;
    typename helper_type::storage_type    storage;
    typename helper_type::bucket_vec_type array;
};

}

template<typename iterator_t>
void radix_sort(iterator_t begin, iterator_t end) {
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;
    typedef detail::radix_sort_helper<value_type> helper_type;

    assert(begin <= end);
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));
    
    thread_pool& p = thread_pool::instance();
    size_t num_threads = p.num_threads();

    size_t aligned_num_elements = align(num_elements, num_threads);
    size_t num_elements_per_thread = aligned_num_elements / num_threads;

    std::vector<per_thread::buckets<value_type> > per_thread_buckets;
    per_thread_buckets.reserve(num_threads);
    for (size_t ii = 0; ii < num_threads; ++ii) {
        per_thread_buckets.emplace_back(num_elements_per_thread);
    }
    
    for(size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        parallel_for_each(begin, end, [&per_thread_buckets, ii](size_t thread_id, value_type& v) {
            per_thread::buckets<value_type>& buckets = per_thread_buckets[thread_id];
            buckets.array[helper_type::digit(ii, v)].push_back(v);
        });

        typename helper_type::storage_type    empty_storage(0);
        typename helper_type::bucket_vec_type joined_buckets(helper_type::num_buckets, empty_storage);

        parallel_for(0, helper_type::num_buckets, [&per_thread_buckets, &joined_buckets](size_t thread_id, size_t digit) {
            for (per_thread::buckets<value_type>& buckets : per_thread_buckets) {
                joined_buckets[digit].squash(std::move(buckets.array[digit]));
            }
        });

        std::vector<std::future<void> > write_futures;
        size_t write_offset = 0;
        size_t current_write_size = 0;
        typedef typename helper_type::bucket_vec_type::iterator bucket_iterator_t;
        bucket_iterator_t this_thread_begin = joined_buckets.begin();
        bucket_iterator_t this_thread_end   = this_thread_begin;

        for (bucket_iterator_t it = joined_buckets.begin(); it != joined_buckets.end(); ++it) {
            size_t bucket_size = it->size();
            current_write_size += bucket_size;

            if (current_write_size >= num_elements_per_thread || (it + 1) == joined_buckets.end()) {
                this_thread_end = it + 1;
                iterator_t write_it = begin + write_offset;
                write_futures.emplace_back(p.async([this_thread_begin, this_thread_end, begin, write_offset]() -> void {
                    iterator_t _write_it = begin + write_offset;
                    for (bucket_iterator_t bucket_it = this_thread_begin; bucket_it != this_thread_end; ++bucket_it) {
                        for (const value_type& value : *bucket_it) {
                            *(_write_it++) = value;
                        }
                    }
                }));

                write_offset += current_write_size;
                current_write_size = 0;
                this_thread_begin = this_thread_end;
            }
        }

        std::for_each(write_futures.begin(), write_futures.end(), [](std::future<void>&f) { f.wait(); });

        for (size_t ii = 0; ii < per_thread_buckets.size(); ++ii)
            per_thread_buckets[ii].storage.reset();
    }
}
}
