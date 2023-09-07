#pragma once

#include "search.hpp"
#include "utils.hpp"

#include <chrono>
#include <cstdint>

constexpr TimePoint marginTimePoint = 5;

struct TimeManagerInitData
{
    TimePoint timeMove;

    TimePoint remainingTime;
    TimePoint timeIncrement;
    TimePoint theirRemainingTime;
    TimePoint theirTimeIncrement;

    std::uint32_t movesToGo;
};

inline void computeTimeLimits(TimeManagerInitData& initData, SearchLimits& searchLimits) {
    // using time control
    if (initData.remainingTime != invalidTimePoint || initData.timeIncrement != invalidTimePoint || initData.timeMove != invalidTimePoint) {
        TimePoint timeToSearch;
        std::uint32_t movesToGo = (initData.movesToGo > 0) ? initData.movesToGo : 40;

        if (initData.timeMove != invalidTimePoint) timeToSearch = initData.timeMove;
        else timeToSearch = (initData.timeIncrement != invalidTimePoint) ? initData.remainingTime / movesToGo + initData.timeIncrement : initData.remainingTime / movesToGo;

        if (timeToSearch > 2 * marginTimePoint) timeToSearch -= marginTimePoint;

        searchLimits.timeLimit = searchLimits.searchTimeStart + timeToSearch;
    }
    else {
        searchLimits.timeLimit = invalidTimePoint;
    }
}
