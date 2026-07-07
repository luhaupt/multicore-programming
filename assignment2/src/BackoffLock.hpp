#pragma once

#include <atomic>
#include <chrono>
#include <random>
#include <thread>

#include "TTASlock.hpp"

class BackoffLock : public TTASlock {
  public:
    BackoffLock(std::size_t min_delay, std::size_t max_delay)
        : min_delay_(min_delay), max_delay_(max_delay) {}

    void lock() override {
        static thread_local std::mt19937_64 generator{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> distribution(min_delay_,
                                                                max_delay_);

        std::size_t delay = min_delay_;

        while (true) {
            while (locked_.load(std::memory_order_relaxed)) {
                active_wait(std::chrono::nanoseconds(delay));

                delay = std::min(delay * 2, max_delay_);
            }

            if (!locked_.exchange(true, std::memory_order_acquire)) {
                return;
            }
        }
    }

    void unlock() override { locked_.store(false, std::memory_order_release); }

  private:
    void active_wait(std::chrono::nanoseconds delay) {
        auto start = std::chrono::high_resolution_clock::now();

        while (std::chrono::high_resolution_clock::now() - start < delay) {
        }
    }

    std::atomic<bool> locked_{false};

    std::size_t min_delay_;
    std::size_t max_delay_;
};
