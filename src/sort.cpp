#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <iterator>

#include "radix-sort.hpp"

struct sorter_base {
    virtual void do_sort() const = 0;
    virtual ~sorter_base() {}
};

template<typename arithm_t>
struct sorter : sorter_base {
    virtual void do_sort() const override {
        size_t size = 0;
        std::cin >> size;
        
        std::vector<arithm_t> vec_to_sort(size);
        std::copy_n(std::istream_iterator<arithm_t>(std::cin), size,  vec_to_sort.begin());

        SORT(vec_to_sort.begin(), vec_to_sort.end());
        
        std::cout << size << std::endl;
        std::copy(vec_to_sort.begin(), vec_to_sort.end(), std::ostream_iterator<arithm_t>(std::cout, " "));
        std::cout << std::endl;
    }
};

int main(int argc, char** argv) {
    const std::map<std::string, sorter_base* > sorters {
        { "uint8_t",  new sorter<uint8_t>()  },
        { "uint16_t", new sorter<uint16_t>() },
        { "uint32_t", new sorter<uint32_t>() },
        { "uint64_t", new sorter<uint64_t>() }
    };
    std::string arithm = "uint32_t";
    if (argc > 1) {
        arithm = argv[1];
    }
    auto found = sorters.find(arithm);
    if (sorters.end() == found) {
        std::cerr << "Arithmetics " << arithm << " is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    found->second->do_sort();
    std::for_each(sorters.begin(), sorters.end(), [] (const std::pair<std::string, sorter_base*>& p) { delete p.second; } );
    exit(EXIT_SUCCESS);
}
