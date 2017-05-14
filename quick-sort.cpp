#include <iostream>
#include <iterator>
#include <vector>

#include <cassert>
#include <cstdlib>

int main(int argc, char** argv) {
    int size = 0;
    std::cin >> size;
    assert(size);
    
    std::vector<int> v(size);
    std::istream_iterator<int> input(std::cin);
    std::copy(input, std::istream_iterator<int>(), v.begin());

    std::sort(v.begin(), v.end());
    
    std::cout << size << std::endl;
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    
    exit(EXIT_SUCCESS);
}

















