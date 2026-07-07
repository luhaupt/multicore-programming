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
        // Number of tree nodes needed for a binary tree
        treeSize_ = 1;
        while (treeSize_ < threads_) {
            treeSize_ *= 2;
        }

        nodes_ = std::make_unique<Node[]>(treeSize_ * 2);

        // Each thread starts at its leaf
        paths_ = std::make_unique<Path[]>(threads_);

        for (std::size_t t = 0; t < threads_; ++t) {
            std::size_t node = treeSize_ + t;

            while (node > 1) {
                node /= 2;
                paths_[t].nodes.push_back(node);
            }
        }
    }

    void lock() override {
        std::size_t id = threadId();

        // Traverse from leaf to root
        for (auto node : paths_[id].nodes) {

            bool expected = false;

            while (!nodes_[node].locked.compare_exchange_weak(
                expected, true, std::memory_order_acquire)) {

                expected = false;
            }
        }
    }

    void unlock() override {
        std::size_t id = threadId();

        // Release from root back down
        for (auto it = paths_[id].nodes.rbegin(); it != paths_[id].nodes.rend();
             ++it) {

            nodes_[*it].locked.store(false, std::memory_order_release);
        }
    }

  private:
    struct alignas(CACHE_LINE_SIZE) Node {
        std::atomic<bool> locked{false};
    };

    struct Path {
        std::vector<std::size_t> nodes;
    };

    std::unique_ptr<Node[]> nodes_;

    std::unique_ptr<Path[]> paths_;

    std::size_t threads_;

    std::size_t treeSize_;

    static std::size_t threadId() {
        static std::atomic<std::size_t> counter{0};
        thread_local std::size_t id = counter.fetch_add(1);
        return id;
    }
};
