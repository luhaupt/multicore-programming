#pragma once

#include "CacheLine.hpp"
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

#include "Lock.hpp"

// Anderson's array-based queue lock: threads spin on their own slot,
// avoiding false sharing since each slot is padded to a cache line.
class ALock : public Lock {
  public:
    explicit ALock(std::size_t threads) : flags_(threads), size_(threads) {
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

    std::vector<AlignedFlag> flags_;

    std::size_t size_;

    std::atomic<std::size_t> next_{0};

    // Shared across all ALock instances on this thread; do not use
    // multiple instances concurrently on the same thread.
    static inline thread_local std::size_t mySlot_;
};
