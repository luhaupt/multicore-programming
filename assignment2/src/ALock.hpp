#pragma once

#include "CacheLine.hpp"
#include <atomic>
#include <cstddef>
#include <memory>

#include "Lock.hpp"

class ALock : public Lock {
  public:
    explicit ALock(std::size_t threads)
        : flags_(std::make_unique<AlignedFlag[]>(threads)), size_(threads) {
        for (std::size_t i = 0; i < size_; ++i) {
            flags_[i].flag.store(false);
        }

        flags_[0].flag.store(true);
    }

    void lock() override {
        mySlot_ = next_.fetch_add(1) % size_;

        while (!flags_[mySlot_].flag.load(std::memory_order_acquire)) {
        }

        flags_[mySlot_].flag.store(false, std::memory_order_release);
    }

    void unlock() override {
        flags_[(mySlot_ + 1) % size_].flag.store(true,
                                                 std::memory_order_release);
    }

  private:
    struct alignas(CACHE_LINE_SIZE) AlignedFlag {
        std::atomic<bool> flag{false};
    };

    std::unique_ptr<AlignedFlag[]> flags_;

    std::size_t size_;

    std::atomic<std::size_t> next_{0};

    static inline thread_local std::size_t mySlot_;
};
