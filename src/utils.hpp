#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>

#define ASSERT(x) {if (!(x)) { std::cout << "Assertion failed: " << #x << std::endl; } }
#define WARNING(x) { std::cout << "WARNING: " << x; }

constexpr std::uint8_t popCount(std::uint64_t x) { return __builtin_popcountll(x); }
constexpr std::uint8_t getLsbIndex(std::uint64_t x) { return __builtin_ctzll(x); }

constexpr std::uint8_t maxSearchDepth = 255;

using Score = std::int16_t;
using ScoreTT = std::int16_t;
constexpr Score invalidScore         = INT16_MAX;
constexpr Score infValue             = 32700;
constexpr Score checkmateValue       = 32000;
constexpr Score checkmateInMaxPly    = checkmateValue - maxSearchDepth;
constexpr Score drawValue            = 0;


using TimePoint = std::chrono::milliseconds::rep;
constexpr TimePoint invalidTimePoint = -1;
inline TimePoint getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
