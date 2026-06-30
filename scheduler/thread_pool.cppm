module;
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

export module scheduler.thread_pool;

export class ThreadPool {
public:
    explicit ThreadPool(std::size_t num_threads) {
        if (num_threads == 0)
            throw std::invalid_argument("ThreadPool requires at least one thread");
        workers_.reserve(num_threads);
        for (std::size_t i = 0; i < num_threads; ++i)
            workers_.emplace_back([this] { worker_loop(); });
    }

    ~ThreadPool() {
        {
            std::lock_guard lock(mutex_);
            stop_.store(true, std::memory_order_release);
        }
        cv_.notify_all();
        for (auto& t : workers_)
            t.join();
    }

    template <typename F>
    std::future<void> submit(F&& f) {
        auto task = std::make_shared<std::packaged_task<void()>>(std::forward<F>(f));
        std::future<void> fut = task->get_future();
        {
            std::lock_guard lock(mutex_);
            if (stop_.load(std::memory_order_acquire))
                throw std::runtime_error("submit on stopped ThreadPool");
            tasks_.push([t = std::move(task)] { (*t)(); });
            active_tasks_.fetch_add(1, std::memory_order_relaxed);
        }
        cv_.notify_one();
        return fut;
    }

    void wait_all() {
        std::unique_lock lock(mutex_);
        idle_cv_.wait(lock, [this] {
            return tasks_.empty() &&
                   active_tasks_.load(std::memory_order_acquire) == 0;
        });
    }

    std::size_t thread_count() const { return workers_.size(); }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this] {
                    return stop_.load(std::memory_order_acquire) || !tasks_.empty();
                });
                if (stop_.load(std::memory_order_acquire) && tasks_.empty())
                    return;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
            if (active_tasks_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                std::lock_guard lock(mutex_);
                idle_cv_.notify_all();
            }
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable idle_cv_;
    std::atomic<bool> stop_{false};
    std::atomic<std::size_t> active_tasks_{0};
};
