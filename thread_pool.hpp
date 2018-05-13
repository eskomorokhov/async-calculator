#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

#include <boost/optional.hpp>

template <typename Object>
class ThreadSafeQueue{
public:
    ThreadSafeQueue(): stop_(false) {}
    boost::optional<Object> pop() {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            condition_.wait(lock,
                                  [this]{ return this->stop_ || !this->queue_.empty();
                                  });
            if (stop_ && queue_.empty()) {
                return boost::none;
            }
            boost::optional<Object> opt_obj{std::move(queue_.front())};
            queue_.pop();
            return std::move(opt_obj);
    }
    void push(Object&& object) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_.emplace(std::move(object));
        }
        condition_.notify_one();
    }
    void stop() {
        stop_ = true;
        condition_.notify_all();
    }
private:
    std::queue<Object> queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};

class ThreadPool {
public:
    typedef std::function<void()> Task;
public:
    ThreadPool(std::size_t);
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

private:
    std::vector< std::thread > workers_;
    std::atomic<bool> stop_;
    ThreadSafeQueue<Task> tasks_;
};

ThreadPool::ThreadPool(std::size_t threads) : stop_(false) {
    for(std::size_t i = 0; i < threads; ++i) {
        workers_.emplace_back(
                [this] {
                    for(; !this->stop_; ) {
                         auto task = tasks_.pop();
                         if (task) {
                             (*task)();
                         }
                    }
                }
        );
    }
}

template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();
    if (stop_) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    tasks_.push([task](){ (*task)(); std::cerr << "finished task\n";});
    return std::move(res);
}

ThreadPool::~ThreadPool() {
    stop_ = true;
    tasks_.stop();
    for (auto &worker : workers_) {
        worker.join();
    }
}