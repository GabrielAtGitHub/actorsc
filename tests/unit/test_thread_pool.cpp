#include <gtest/gtest.h>
#include <atomic>
#include <future>
#include <stdexcept>
#include <vector>

import scheduler.thread_pool;

TEST(ThreadPool, ExecutesSubmittedTask) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    auto f = pool.submit([&] { counter.fetch_add(1, std::memory_order_relaxed); });
    f.get();
    EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadPool, ExecutesMultipleTasksConcurrently) {
    ThreadPool pool(4);
    constexpr int N = 100;
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futs;
    futs.reserve(N);
    for (int i = 0; i < N; ++i)
        futs.push_back(pool.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    for (auto& f : futs)
        f.get();
    EXPECT_EQ(counter.load(), N);
}

TEST(ThreadPool, WaitAllBlocksUntilIdle) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    for (int i = 0; i < 10; ++i)
        pool.submit([&] { counter.fetch_add(1, std::memory_order_relaxed); });
    pool.wait_all();
    EXPECT_EQ(counter.load(), 10);
}

TEST(ThreadPool, RejectsZeroThreads) {
    EXPECT_THROW(ThreadPool{0}, std::invalid_argument);
}

TEST(ThreadPool, ThreadCountMatchesConstructorArg) {
    ThreadPool pool(3);
    EXPECT_EQ(pool.thread_count(), 3u);
}
