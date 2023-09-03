#pragma once

#include <cstdint>

constexpr std::uint8_t popCount(std::uint64_t x) { return __builtin_popcountll(x); }
constexpr std::uint8_t getLsbIndex(std::uint64_t x) { return __builtin_ctzll(x); }
