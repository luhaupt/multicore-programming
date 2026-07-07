#pragma once
#include "CacheLine.hpp"
#include "Lock.hpp"
#include <atomic>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

class ALog : public Lock {
  public:
    explicit ALog(std::size_t threads) : threads_(threads) {
        if (threads_ <= 1)
            return;
        levels_ = static_cast<std::size_t>(std::ceil(std::log2(threads_)));
        tree_.resize(levels_);
        std::size_t nodes = threads_;
        for (std::size_t level = 0; level < levels_; level++) {
            nodes = (nodes + 1) / 2;
            tree_[level] = std::make_unique<Node[]>(nodes);
        }
        paths_.resize(threads_);
        for (std::size_t t = 0; t < threads_; t++) {
            std::size_t node = t;
            for (std::size_t level = 0; level < levels_; level++) {
                paths_[t].nodes.push_back(node / 2);
                paths_[t].side.push_back(node % 2);
                node /= 2;
            }
        }
    }

    void lock() override {
        if (threads_ <= 1)
            return;
        if (mySlot_ == kUnset) {
            mySlot_ = next_.fetch_add(1, std::memory_order_relaxed) % threads_;
        }
        auto &path = paths_[mySlot_];
        for (std::size_t level = 0; level < levels_; level++) {
            auto node = path.nodes[level];
            int side = static_cast<int>(path.side[level]);
            auto &n = tree_[level][node];

            n.flag[side].store(true, std::memory_order_seq_cst);
            n.turn.store(1 - side, std::memory_order_seq_cst);
            while (n.flag[1 - side].load(std::memory_order_seq_cst) &&
                   n.turn.load(std::memory_order_seq_cst) == 1 - side) {
                // spin
            }
        }
    }

    void unlock() override {
        if (threads_ <= 1)
            return;
        auto &path = paths_[mySlot_];
        for (std::size_t level = levels_; level-- > 0;) {
            auto node = path.nodes[level];
            int side = static_cast<int>(path.side[level]);
            tree_[level][node].flag[side].store(false,
                                                std::memory_order_seq_cst);
        }
    }

  private:
    struct alignas(CACHE_LINE_SIZE) Node {
        std::atomic<bool> flag[2] = {false, false};
        std::atomic<int> turn{0};
    };
    struct Path {
        std::vector<std::size_t> nodes;
        std::vector<std::size_t> side;
    };

    std::vector<std::unique_ptr<Node[]>> tree_;
    std::vector<Path> paths_;
    std::size_t threads_;
    std::size_t levels_{0};
    std::atomic<std::size_t> next_{0};

    static constexpr std::size_t kUnset =
        std::numeric_limits<std::size_t>::max();
    static inline thread_local std::size_t mySlot_{kUnset};
};
