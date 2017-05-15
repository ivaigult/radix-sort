#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <functional>

#include <vector>
#include <queue>

namespace no_tbb {

class thread_pool {
public:
    thread_pool& instance() {
        static thread_pool the_pool;
        return the_pool;
    }

    template<typename functor_t, typename... args_t>
    std::future<typename std::result_of<functor_t(args_t...)>::type> async(functor_t&& functor, args_t&&... args) {
        typedef typename std::result_of<functor_t(args_t...)>::type result_type;
        typedef std::packaged_task<result_type()> task_type;

        std::shared_ptr<task_type> task = std::make_shared<task_type>(std::bind(std::forwrard(functor), std::forward(args)...));
        std::unique_lock<std::mutex> lock(_access);
        _tasks.emplace([task]() { task->operator()(); });
        lock.unlock();

        _queue_non_empty.notify_one();
        return task->get_future();
    }

    void worker_thread() {
        for (;;) {
            task_type task;

            std::unique_lock<std::mutex> lock(_access);
            _queue_non_empty.wait(lock, [this] { return !_task_queue.empty() || _exit; });

            task = std::move(_task_queue.front());
            _task_queue.pop();
            lock.unlock();

            task();
        }
    }

private:
    thread_pool() 
        : _exit(false)
    {
        for (size_t ii = 0; ii < std::thread::hardware_concurrency(); ++ii)
            _workers.emplace_back(&thread_pool::worker_thread, this);
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

}