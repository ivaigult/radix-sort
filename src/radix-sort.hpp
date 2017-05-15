#pragma once

#include <iterator>
#include <limits>
#include <cstdint>
#include <array>
#include <deque>

namespace detail {

template<typename value_t>
struct bucket_node {
    value_t               value;
    bucket_node<value_t>* tail;
};

template<typename value_t>
struct bucket_storage {
    bucket_storage(size_t num_elements)
        : _num_allocated(0)
        , _data(num_elements)
    {}

    bucket_node<value_t>* alloc() { return &_data[_num_allocated++]; }
    void                  reset() { _num_allocated = 0; }
private:
    size_t                             _num_allocated;
    std::vector<bucket_node<value_t> > _data;
};
    
template<typename value_t>
struct bucket {
    typedef value_t value_type;

    bucket(bucket_storage<value_t>& s)
        : _storage(s)
        , _front(nullptr)
        , _back(nullptr)
    {}
    
    void push_back(value_type v) {
        bucket_node<value_type>* new_node = _storage.alloc();
        new_node->value = v;
        new_node->tail = nullptr;
        if (!_front) {
            _front = new_node;
        }

        if (!_back) {
            _back = new_node;
        } else {
            _back->tail = new_node;
            _back = new_node;
        }
    }

    void clear() {
        _front = nullptr;
        _back = nullptr;
    }

    struct iterator {
        iterator(bucket_node<value_type>* node) : _node(node) {}
        iterator&    operator=(const iterator& it) { _node = it._node; return *this; }
        bool         operator==(const iterator& it) const { return _node == it._node; }
        bool         operator!=(const iterator& it) const { return _node != it._node; }
        iterator&    operator++()    { _node = static_cast<bucket_node<value_type>*>(_node->tail); return *this; }
        iterator     operator++(int) {
            bucket_node<value_type>* prev_node = _node;
            _node = static_cast<bucket_node<value_type>*>(_node->tail);
            return prev_node;
        }
        value_type   operator*() const { return _node->value; }
    private:
        bucket_node<value_type>* _node;
    };

    iterator begin() { return _front; }
    iterator end()   { return nullptr; }
    
private:
    bucket_storage<value_t>& _storage;
    bucket_node<value_type>* _front;
    bucket_node<value_type>* _back;
};
    
template<typename value_t, typename radix_t = uint8_t>
struct radix_sort_helper {
    typedef value_t value_type;
    typedef radix_t radix_type;
    enum { num_digits = sizeof(value_type) / sizeof(radix_type) };
    enum { bits_per_digit = sizeof(radix_type) * 8 };    

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
    
    size_t num_elements = std::distance(begin, end);
    detail::bucket_storage<value_type> storage(num_elements);
    std::vector<detail::bucket<value_type> > array(std::numeric_limits<typename helper_type::radix_type>::max() + 1, detail::bucket<value_type>(storage)); 
    
    for(size_t ii = 0; ii < helper_type::num_digits; ++ii) {
        for(iterator_t it = begin; it != end; ++it) {
            array[helper_type::digit(ii, *it)].push_back(*it);
        }

        iterator_t it = begin;
        for(auto& bucket : array) {
            for(const value_type& value: bucket) {
                *(it++) = value;
            }
            bucket.clear();
        }
        storage.reset();
    }
}

