#pragma once

#include <atomic>

#include "CacheLine.hpp"
#include "Lock.hpp"

struct alignas(CACHE_LINE_SIZE) QNode {
    std::atomic<QNode *> next{nullptr};
    std::atomic<bool> locked{false};
};

class MCSLock : public Lock {
  public:
    void lock() override {
        myNode.next.store(nullptr, std::memory_order_relaxed);
        QNode *predecessor = tail.exchange(&myNode, std::memory_order_acq_rel);

        if (predecessor != nullptr) {
            myNode.locked.store(true, std::memory_order_relaxed);
            predecessor->next.store(&myNode, std::memory_order_release);

            while (myNode.locked.load(std::memory_order_acquire)) {
            }
        }
    }

    void unlock() override {
        QNode *successor = myNode.next.load(std::memory_order_acquire);

        if (successor == nullptr) {
            QNode *expected = &myNode;

            if (tail.compare_exchange_strong(expected, nullptr,
                                             std::memory_order_acq_rel)) {
                return;
            }

            while ((successor = myNode.next.load(std::memory_order_acquire)) ==
                   nullptr) {
            }
        }

        successor->locked.store(false, std::memory_order_release);
        myNode.next.store(nullptr, std::memory_order_relaxed);
    }

  private:
    std::atomic<QNode *> tail{nullptr};
    static inline thread_local QNode myNode;
};
