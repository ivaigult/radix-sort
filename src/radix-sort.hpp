#pragma once

#include <limits>   // std::numeric_limits<...>::max()
#include <cstdint>  // std::uint8_t
#include <iterator> // std::iterator_traits<...>::value_type
#include <vector>   // std::vector

#include <cassert>  

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
    {}
    
    void push_back(value_type v) {
        node_type* new_node = _storage.alloc();
        new_node->value = v;
        new_node->tail = nullptr;
        if (!_front) {
            assert(!_back);
            _front = _back = new_node;
        }

        _back->tail = new_node;
        _back       = new_node;
    }

    void clear() {
        _front = nullptr;
        _back = nullptr;
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

