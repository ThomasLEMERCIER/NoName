#pragma once

#include "move.hpp"
#include "movelist.hpp"
#include "position.hpp"

#include <cstdint>

enum class MoveSorterStage : std::uint8_t {
    TTMove,
    GeneratingNonQuiets,
    NonQuiets,
    GeneratingQuiets,
    Quiets
};

class MoveSorter {
private:
    MoveList moveList;
    const Position& position;
    const Move ttMove;
    MoveSorterStage currentStage = MoveSorterStage::TTMove;
    std::uint32_t indexMoveList = 0;
    bool onlyNonQuiets = false;

    void scoreNonQuiets();
    Move nextSortedMove();
public:
    bool nextMove(Move& outMove);

    MoveSorter(const Position& pos, const Move& move) : position{pos}, ttMove{move} {};
    MoveSorter(const Position& pos, const Move& move, bool inQuiescence) : position{pos}, ttMove{move}, onlyNonQuiets{inQuiescence} {};
};