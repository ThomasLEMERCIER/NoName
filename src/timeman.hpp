#pragma once

#include <chrono>

using TimePoint = std::chrono::milliseconds::rep;

inline TimePoint get_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
