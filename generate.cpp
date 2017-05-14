#include <iostream>
#include <random>
#include <string>
#include <cstdlib>

int main(int argc, char** argv) {
    if (4 != argc) {
        std::cerr << "Usage: " << argv[0] << " <count> <min> <max>";
        exit(EXIT_FAILURE);
    }

    int size = std::stoi(argv[1]);
    int min  = std::stoi(argv[2]);
    int max  = std::stoi(argv[3]);
    
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uniform(min,max);

    std::cout << size << std::endl;
    for(int ii = 0; ii < size; ++ii) {
        std::cout << uniform(rng) << " ";
    }
    std::cout << std::endl;
    
    exit(EXIT_SUCCESS);
}










