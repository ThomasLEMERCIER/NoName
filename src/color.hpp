#pragma once

#include <cstdint>
#include <string_view>

enum class Color : std::uint8_t {
    White,
    Black,
};

constexpr Color operator~(const Color color) {
    return static_cast<Color>(static_cast<std::uint8_t>(color) ^ 1);
}
