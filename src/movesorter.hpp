#pragma once

#include "move.hpp"
#include "movelist.hpp"
#include "position.hpp"

#include <cstdint>

enum class MoveSorterStage : std::uint8_t {
    GeneratingNonQuiets,
    NonQuiets,
    GeneratingQuiets,
    Quiets
};

class MoveSorter {
private:
    MoveList moveList;
    const Position& position;
    MoveSorterStage currentStage = MoveSorterStage::GeneratingNonQuiets;
    std::uint32_t indexMoveList = 0;
    bool onlyNonQuiets = false;
public:
    bool nextMove(Move& outMove);

    explicit MoveSorter(const Position& pos) : position{pos} {};
    MoveSorter(const Position& pos, bool inQuiescence) : position{pos}, onlyNonQuiets{inQuiescence} {};
};