#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <utility>

#include <vector>
#include <queue>

#if defined(_WIN32)
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include "windows.h"
#else
#  include <pthread.h>
#endif

namespace no_tbb {

namespace __os {
void set_affinity(std::thread& thread, size_t core_index);
#if defined(_WIN32)
void set_affinity(std::thread& thread, size_t core_index) {
    DWORD_PTR affinity_mask = static_cast<DWORD_PTR>(1) << core_index;
    SetThreadAffinityMask(thread.native_handle(), affinity_mask);
}
#else
void set_affinity(std::thread& thread, size_t core_index) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_index, &cpuset);
    pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
}
#endif
}

class thread_pool {
public:
    static thread_pool& instance() {
        static thread_pool the_pool;
        return the_pool;
    }

    template<typename functor_t, typename... args_t>
    std::future<typename std::result_of<functor_t(args_t...)>::type> async(functor_t&& functor, args_t&&... args) {
        typedef typename std::result_of<functor_t(args_t...)>::type result_type;
        typedef std::packaged_task<result_type()> task_type;

        std::shared_ptr<task_type> task = std::make_shared<task_type>(std::bind(std::forward<functor_t>(functor), std::forward<args_t>(args)...));
        std::unique_lock<std::mutex> lock(_access);
        _task_queue.emplace([task]() { task->operator()(); });
        lock.unlock();

        _queue_non_empty.notify_one();
        return task->get_future();
    }

    void worker_thread() {
        for (;;) {
            task_type task;

            std::unique_lock<std::mutex> lock(_access);
            _queue_non_empty.wait(lock, [this] { return !_task_queue.empty() || _exit; });
            if (_exit) {
                break;
            }
                
            task = std::move(_task_queue.front());
            _task_queue.pop();
            lock.unlock();

            task();
        }
    }

    size_t num_threads() { return _workers.size();  }

private:
    thread_pool() 
        : _exit(false)
    {
        for (size_t ii = 0; ii < std::thread::hardware_concurrency(); ++ii) {
            _workers.emplace_back(&thread_pool::worker_thread, this);
            __os::set_affinity(_workers.back(), ii);
        }
    }
    ~thread_pool() {
        std::unique_lock<std::mutex> lock(_access);
        _exit = true;
        lock.unlock();
        _queue_non_empty.notify_all();

        for (std::thread& t : _workers) {
            t.join();
        }
    }

    typedef std::function<void(void)>  task_type;
    std::queue<task_type>    _task_queue;
    std::mutex               _access;
    std::condition_variable  _queue_non_empty;
    std::vector<std::thread> _workers;
    bool                     _exit;
};

size_t align(size_t value, size_t unit) {
    return (value + unit - 1) / unit * unit;
}

template<typename iterator_t, typename functor_t>
void parallel_for_each(iterator_t begin, iterator_t end, functor_t&& functor) {
    typedef typename std::iterator_traits<iterator_t>::value_type value_type;

    thread_pool& p = thread_pool::instance();
    size_t num_threads = p.num_threads();
    size_t num_elements = static_cast<size_t>(std::distance(begin, end));
    size_t aligned_num_elements = align(num_elements, num_threads);
    size_t num_elements_per_thread = aligned_num_elements / num_threads;

    std::vector<std::future<void> > futures;
    for (size_t thread_id = 0; thread_id < num_threads; ++thread_id) {
        iterator_t this_thread_begin = begin + num_elements_per_thread * thread_id;
        iterator_t this_thread_end   = begin + std::min(num_elements_per_thread * (1 + thread_id), num_elements);

        futures.emplace_back(p.async([this_thread_begin, this_thread_end, thread_id, functor]() -> void {
            for (iterator_t it = this_thread_begin; it != this_thread_end; ++it) {
                functor(thread_id, *it);
            }
        }));
    }

    for (std::future<void>& f : futures) {
        f.wait();
    }
}

template<typename functor_t>
void parallel_for(size_t begin, size_t end, functor_t&& functor) {
    thread_pool& p = thread_pool::instance();
    size_t num_threads = p.num_threads();
    size_t num_elements = static_cast<size_t>(end - begin);
    size_t aligned_num_elements = align(num_elements, num_threads);
    size_t num_elements_per_thread = aligned_num_elements / num_threads;

    std::vector<std::future<void> > futures;
    for (size_t thread_id = 0; thread_id < num_threads; ++thread_id) {
        size_t this_thread_begin = begin + num_elements_per_thread * thread_id;
        size_t this_thread_end   = begin + std::min(num_elements_per_thread * (1 + thread_id), end);

        futures.emplace_back(p.async([this_thread_begin, this_thread_end, thread_id, functor]() -> void {
            for (size_t ii = this_thread_begin; ii != this_thread_end; ++ii) {
                functor(thread_id, ii);
            }
        }));
    }

    for (std::future<void>& f : futures) {
        f.wait();
    }
}
 
}