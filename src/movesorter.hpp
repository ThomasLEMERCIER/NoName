#pragma once

#include "move.hpp"
#include "movelist.hpp"
#include "position.hpp"

#include <array>
#include <cstdint>

enum class MoveSorterStage : std::uint8_t {
    TTMove,
    GeneratingNonQuiets,
    NonQuiets,
    GeneratingQuiets,
    Quiets
};

constexpr std::int32_t historyBound = 32000;
using MoveHistoryTable = std::array<std::array<std::array<std::int32_t, 64>, 64>, 2>;

class MoveSorter {
private:
    MoveList moveList;
    const Position& position;

    const Move ttMove;
    const MoveHistoryTable& quietHistoryTable;

    MoveSorterStage currentStage = MoveSorterStage::TTMove;
    std::uint32_t indexMoveList = 0;

    void scoreNonQuiets();
    void scoreQuiets();

    Move nextSortedMove();
public:
    bool nextMove(Move& outMove, bool skipQuiet);

    MoveSorter(const Position& pos, const Move& move, const MoveHistoryTable& historyTable) : position{pos}, ttMove{move}, quietHistoryTable{historyTable} {};
};