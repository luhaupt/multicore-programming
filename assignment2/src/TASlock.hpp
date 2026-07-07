#pragma once
#include "Lock.hpp"
#include <atomic>

class TASlock : public Lock {
  protected:
    std::atomic_flag state = false;

  public:
    void lock() override {
        while (state.test_and_set()) {
        }
    }

    void unlock() override { state.clear(); }
};
