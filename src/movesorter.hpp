#pragma once

#include "move.hpp"
#include "movelist.hpp"
#include "position.hpp"

#include <array>
#include <cstdint>

enum class MoveSorterStage : std::uint8_t {
    TTMove,
    GeneratingNonQuiets,
    GoodNonQuiets,
    GeneratingQuiets,
    Killer1,
    Killer2,
    CounterMove,
    OrderingQuiets,
    Quiets,
    BadNonQuiets,
};

struct KillerMoves {
    Move killer1 = Move::Invalid();
    Move killer2 = Move::Invalid();

    constexpr void clear() {
        killer1 = Move::Invalid();
        killer2 = Move::Invalid();
    }
};

using MoveHistoryTable = std::array<std::array<std::array<std::int32_t, 64>, 64>, 2>;
using KillerMoveTable = std::array<KillerMoves, maxSearchDepth>;
using CounterMoveTable = std::array<std::array<Move, 64>, 12>;

class MoveSorter {
private:
    MoveList moveList;
    const Position& position;

    const Move ttMove;
    const MoveHistoryTable& quietHistoryTable;
    const KillerMoves& killerMoves;
    const Move counterMove;

    MoveSorterStage currentStage = MoveSorterStage::TTMove;

    std::uint32_t currentIndex      = 0;
    std::uint32_t badNonQuietsIndex = 0;
    std::uint32_t quietMoveIndex    = 0;

    void scoreNonQuiets();
    void scoreQuiets();

    std::uint32_t nextSortedIndex(std::uint32_t start, std::uint32_t end) ;
    Move pop(std::uint32_t index);

    constexpr static std::int32_t GoodNonQuietThreshold = -103;
    constexpr static std::uint32_t InvalidIndex = UINT32_MAX;

    constexpr static std::int32_t GoodNonQuietScoreBonus = 50000;
    constexpr static std::int32_t GoodNonQuietScore = 40000;


public:
    bool nextMove(Move& outMove, bool skipQuiet, bool skipBadNonQuiet);

    MoveSorter(const Position& pos, const Move& move, const MoveHistoryTable& historyTable, const KillerMoves& killers, const Move& counter) : position{pos}, ttMove{move}, quietHistoryTable{historyTable}, killerMoves{killers}, counterMove{counter} {};
};