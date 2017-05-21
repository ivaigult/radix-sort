#include <algorithm>
#include <chrono>
#include <vector>
#include <limits>
#include <iterator>
#include <functional>
#include <random>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <map>

#include <radix_sort/sort.hpp>
#include <radix_sort/concurrent_sort.hpp>
#include <radix_sort/tbb_concurrent_sort.hpp>

template<typename value_t>
struct msvc_rnd_workaround {
    typedef value_t type;
};

#if defined(_MSC_VER )
// http://connect.microsoft.com/VisualStudio/feedbackdetail/view/856484/std-uniform-int-distribution-does-not-like-char
template<>
struct msvc_rnd_workaround<uint8_t>
{
    typedef uint16_t type;
};
#endif

#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20120322)
typedef std::chrono::monotonic_clock steady_clock;
#else
typedef std::chrono::steady_clock steady_clock;
#endif

template<typename value_t>
struct experiment {

    experiment(size_t size)
        : _random_device()
        , _mersenne_twister(_random_device())
        , _uniform(std::numeric_limits<value_t>::min(), std::numeric_limits<value_t>::max())
        , _unsorted([size, this]() {
            std::vector<value_t> result;
            result.reserve(size);
            std::generate_n(std::back_inserter(result), size, std::bind(_uniform, _mersenne_twister));
            return std::move(result);
        }())
        , _gold_sorted([this]() {
            std::vector<value_t> result = _unsorted;
            std::sort(result.begin(), result.end());
            return std::move(result);
        }())
        , _std_sort_msec                  (_sorting_experiment(std::sort                  <iterator_type>))
        , _radix_sort_sort_msec           (_sorting_experiment(radix_sort::sort           <iterator_type>))
        , _radix_sort_concurrent_sort_msec(_sorting_experiment(radix_sort::concurrent_sort<iterator_type>))
    {}

    experiment(const experiment<value_t>&) = delete;
    experiment<value_t>& operator=(const experiment<value_t>&) = delete;

private:
    typedef std::vector<value_t> value_vec_t;
    typedef typename std::vector<value_t>::iterator iterator_type;
    
    uint64_t _sorting_experiment(void (*algorithm)(iterator_type begin, iterator_type end) ) {
        value_vec_t sorted = _unsorted;

        std::chrono::time_point<steady_clock> begin = steady_clock::now();
        algorithm(sorted.begin(), sorted.end());
        std::chrono::time_point<steady_clock> end   = steady_clock::now();

        if (sorted != _gold_sorted) {
            throw std::logic_error("An implementation gave different results than std::sort");
        }
        
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    }

    typedef typename msvc_rnd_workaround<value_t>::type rnd_value_type;

    std::random_device                            _random_device;
    std::mt19937                                  _mersenne_twister;
    std::uniform_int_distribution<rnd_value_type> _uniform;
    
    const value_vec_t _unsorted;
    const value_vec_t _gold_sorted;

    const uint64_t    _std_sort_msec;
    const uint64_t    _radix_sort_sort_msec;
    const uint64_t    _radix_sort_concurrent_sort_msec;


    template<typename other_value_t>
    friend std::ostream& operator<<(std::ostream &os, const experiment<other_value_t>& e);
};


template<typename value_t>
std::ostream& operator<<(std::ostream& os, const experiment<value_t>& e) {
    os << std::left << 
        std::setw(15) << e._std_sort_msec <<
        std::setw(15) << e._radix_sort_sort_msec <<
        std::setw(15) << e._radix_sort_concurrent_sort_msec;
    return os;
}

struct benchmark_base {
    virtual ~benchmark_base() {}
    virtual void go(size_t start, size_t stop, size_t step) = 0;
};

template<typename value_t>
struct benchmark : public benchmark_base {
    virtual void go(size_t start, size_t stop, size_t step) {
        std::cout << std::left << 
            std::setw(15) << "std::sort" <<
            std::setw(15) << "radix_sort" <<
            std::setw(15) << "concurrent_radix_sort" <<
            std::setw(15) << "tbb_radix_concurrent_sort" <<
            std::endl;
        for(size_t size = start; size < stop; size += step) {
            const experiment<value_t> e(size);
            std::cout << e << std::endl;
        }
    }
};

size_t to_size_t(const std::string& s) {
    std::stringstream ss(s);
    size_t result = 0;
    ss >> result;
    return result;
}

int main(int argc, char** argv)
try {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <arithm> <start> <stop> <step>" << std::endl;
        throw std::runtime_error("Incorrect number of arguments");
    }
    
    const std::map<std::string, benchmark_base* > benchmarks {
        { "uint8_t",  new benchmark<uint8_t>()  },
        { "uint16_t", new benchmark<uint16_t>() },
        { "uint32_t", new benchmark<uint32_t>() },
        { "uint64_t", new benchmark<uint64_t>() }
    };

    std::string arithm = argv[1];
    size_t start = to_size_t(argv[2]);
    size_t stop  = to_size_t(argv[3]);
    size_t step  = to_size_t(argv[4]);
    
    auto found = benchmarks.find(arithm);
    if (benchmarks.end() == found) {
        throw std::runtime_error("Arithmetic is not supported");
        exit(EXIT_FAILURE);
    }

    found->second->go(start, stop, step);

    std::for_each(benchmarks.begin(), benchmarks.end(), [] (const std::pair<std::string, benchmark_base*>& p) { delete p.second; } );
    exit(EXIT_SUCCESS);
} catch (std::exception& e) {
    // @ fixme: map will leak
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
}










