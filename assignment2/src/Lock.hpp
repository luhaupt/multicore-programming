#pragma once

class Lock {
  public:
    Lock(const Lock &lock) = delete;
    Lock() {}

    virtual void lock() = 0;
    virtual void unlock() = 0;
};
