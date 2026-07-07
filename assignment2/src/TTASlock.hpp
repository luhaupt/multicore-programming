#pragma once
#include "TASlock.hpp"

class TTASlock : public TASlock {
  public:
    void lock() {
        while (true) {
            while (state.test()) {
            }
            if (!state.test_and_set())
                return;
        }
    }
};
