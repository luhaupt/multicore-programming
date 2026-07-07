#pragma once

#include <atomic>
#include <cstddef>

#include "Lock.hpp"

class Ticker {
  public:
    virtual ~Ticker() = default;

    virtual std::size_t operator++() = 0;
    virtual std::size_t operator++(int) = 0;
};

class LockedTicker : public Ticker {
  public:
    explicit LockedTicker(Lock &lock) : lock_(lock), counter_(0) {}

    std::size_t operator++(int) override {
        lock_.lock();
        auto old = counter_;
        counter_ += 1;
        lock_.unlock();
        return old;
    }

    std::size_t operator++() override {
        lock_.lock();
        ++counter_;
        auto value = counter_;
        lock_.unlock();
        return value;
    }

  private:
    Lock &lock_;
    std::size_t counter_ = 0;
};

class STLAtomicTicker : public Ticker {
  public:
    STLAtomicTicker() : counter_(0) {}

    std::size_t operator++(int) override { return counter_.fetch_add(1); }
    std::size_t operator++() override { return counter_.fetch_add(1) + 1; }
    std::size_t load() const { return counter_.load(); }

  private:
    std::atomic_size_t counter_;
};
