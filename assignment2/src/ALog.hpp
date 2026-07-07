#pragma once

#include "CacheLine.hpp"
#include "Lock.hpp"

#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>

class ALog : public Lock {
  public:
    explicit ALog(std::size_t threads) : threads_(threads) {
        if (threads_ <= 1) {
            return;
        }

        levels_ = static_cast<std::size_t>(std::ceil(std::log2(threads_)));
        flags_ = std::make_unique<std::unique_ptr<AlignedFlag[]>[]>(levels_);
        std::size_t size = threads_;

        for (std::size_t level = 0; level < levels_; level++) {
            flags_[level] = std::make_unique<AlignedFlag[]>(size);

            for (std::size_t i = 0; i < size; i++) {
                flags_[level][i].flag.store(false);
            }

            size = (size + 1) / 2;
        }

        flags_[0][0].flag.store(true);
    }

    void lock() override {
        if (threads_ <= 1) {
            return;
        }

        mySlot_ = next_.fetch_add(1) % threads_;
        std::size_t slot = mySlot_;

        for (std::size_t level = 0; level < levels_; level++) {
            std::size_t parent = slot / 2;

            while (
                !flags_[level][parent].flag.load(std::memory_order_acquire)) {
            }

            flags_[level][parent].flag.store(false, std::memory_order_release);
            slot = parent;
        }
    }

    void unlock() override {
        if (threads_ <= 1) {
            return;
        }

        std::size_t slot = mySlot_;

        for (std::size_t level = levels_; level-- > 0;) {
            std::size_t parent = slot / 2;
            flags_[level][parent].flag.store(true, std::memory_order_release);
            slot = parent;
        }
    }

  private:
    struct alignas(CACHE_LINE_SIZE) AlignedFlag {
        std::atomic<bool> flag{false};
    };

    std::unique_ptr<std::unique_ptr<AlignedFlag[]>[]> flags_;
    std::size_t threads_;
    std::size_t levels_{0};
    std::atomic<std::size_t> next_{0};
    static inline thread_local std::size_t mySlot_{0};
};
