#pragma once

#include <cstdint>

constexpr std::uint8_t PopCount(std::uint64_t x) { return __builtin_popcountll(x); }
constexpr std::uint8_t GetLsbIndex(std::uint64_t x) { return __builtin_ctzll(x); }
