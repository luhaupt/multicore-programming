#pragma once

#include <cstddef>

// Assumed cache line size; std::hardware_destructive_interference_size
// (<new>) would be the portable alternative, but its use triggers
// ABI-stability warnings across TUs (hence -Wno-interference-size).
inline constexpr std::size_t CACHE_LINE_SIZE = 64;
