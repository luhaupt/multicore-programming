#pragma once

#include "CacheLine.hpp"
#include "Lock.hpp"
#include <atomic>

struct alignas(CACHE_LINE_SIZE) QNode {
    std::atomic<QNode *> next{nullptr};
    std::atomic<bool> locked{false};
};

// MCS queue lock: each thread spins on its own node (no shared cache line
// contention), forming an implicit queue via `next` pointers.
class MCSLock : public Lock {
  public:
    void lock() override {
        myNode.next.store(nullptr, std::memory_order_relaxed);
        QNode *predecessor = tail.exchange(&myNode, std::memory_order_acq_rel);

        if (predecessor != nullptr) {
            // Someone's ahead of us in the queue; wait for them to hand off.
            myNode.locked.store(true, std::memory_order_relaxed);
            predecessor->next.store(&myNode, std::memory_order_release);

            while (myNode.locked.load(std::memory_order_acquire)) {
            }
        }
    }

    void unlock() override {
        QNode *successor = myNode.next.load(std::memory_order_acquire);

        if (successor == nullptr) {
            // No visible successor yet: try to claim we're the last node.
            // If that fails, a successor is mid-enqueue but hasn't linked
            // itself in yet, so wait for it to appear before continuing.
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

    // Shared across all MCSLock instances on this thread (same caveat as
    // ALock/ALog's thread_local slot).
    static inline thread_local QNode myNode;
};
