#pragma once

class Lock {
  public:
    Lock(const Lock &lock) = delete;
    Lock() = default;
    Lock &operator=(const Lock &) = delete;

    virtual ~Lock() = default;
    virtual void lock() = 0;
    virtual void unlock() = 0;
};
