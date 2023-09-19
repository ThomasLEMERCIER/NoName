#pragma once

#include "color.hpp"
#include "position.hpp"

#include <cstdint>
#include <vector>

class Game {
private:
    std::vector<std::uint64_t> positionHistory;
    Position currentPosition;
    bool valid = false;
public:
    bool checkRepetition(std::uint64_t hash) const;
    void recordPosition(Position& position);
    void reset();
    constexpr Position getCurrentPosition() const { return currentPosition; };
    constexpr bool isValid() const { return valid; };
    constexpr Color getSideToMove() const { return currentPosition.sideToMove; };
};
