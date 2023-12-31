#pragma once

#include "game.hpp"
#include "move.hpp"
#include "movesorter.hpp"
#include "position.hpp"
#include "transpositiontable.hpp"
#include "utils.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <thread>

constexpr Score aspirationWindowStart = 20;
constexpr Score aspirationWindowMinDepth = 5;

constexpr std::int16_t nullMovePruningDepthReduction = 4;
constexpr std::uint16_t nullMovePruningStartDepth = 2;

constexpr std::uint8_t baseReduction = 1;
constexpr double scaleReduction = 0.5;
extern std::uint8_t lateMoveReductionTable[64][64];

constexpr std::int16_t reverseFutilityDepth = 8;
constexpr Score baseFutilityMargin = 10;
constexpr Score scaleFutilityMargin = 75;

constexpr std::uint32_t baseLateMovePruning = 3;
constexpr std::uint32_t scaleLateMovePruning = 8;

constexpr std::int16_t seePruningDepth = 8;
constexpr std::int32_t scaleNonQuietSeePruning = -80;
constexpr std::int32_t scaleQuietSeePruning = -30;


void initSearchParameters();

enum class NodeType: std::uint8_t {
    Root,
    Pv,
    NonPv
};

struct PvLine {
    std::array<Move, maxSearchDepth> moves;
    std::uint8_t pvLength;
    Score score;
};

struct SearchLimits {
    std::uint8_t depthLimit;
    TimePoint searchTimeStart;
    TimePoint timeLimit;
};

struct NodeData {
    Position position;
    bool inCheck;

    Score alpha;
    Score beta;

    std::int16_t depth;
    std::int16_t ply;

    PvLine pvLine;
    Move previousMove;

    void clear() {
        position = {};
        inCheck = {};
        alpha = {};
        beta = {};
        depth = {};
        ply = {};
        pvLine.pvLength = 0;
        previousMove = Move::Invalid();
    }
};

struct SearchStats {
    std::uint64_t negamaxNodeCounter;
    std::uint64_t quiescenceNodeCounter;

#ifdef SEARCH_STATS
    std::uint64_t betaCutoff;
    std::uint64_t ttHits;
#endif
};

struct ThreadData {
    SearchLimits searchLimits;
    const Game* game;

    std::array<NodeData, maxSearchDepth> searchStack;

    bool isMainThread;
    SearchStats searchStats;

    MoveHistoryTable moveHistoryTable;
    KillerMoveTable killerMoveTable;
    CounterMoveTable counterMoveTable;
};

class Search {
public:
    void startSearch(const Game& game, const SearchLimits& searchLimits);
    void stopSearch();
    void searchInternal(ThreadData& threadData);
    void clear() { transpositionTable.clear(); };
    void resizeTT(std::uint64_t newMemorySize) { transpositionTable.initTable(newMemorySize); };
    void setStopSearchFlag(const bool flag) { searchStop = flag; };

private:
    static void reportInfo(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats);
    static void reportResult(Move bestMove);
    bool checkStopCondition(SearchLimits& searchLimits, SearchStats& searchStats);

    static bool isRepetition(NodeData* nodeData, const Game* game);
    static constexpr Score futilityMargin(std::int16_t depth);
    static constexpr std::uint32_t lateMovePruningThreshold(std::int16_t depth);
    static void updateQuietMoveOrdering(ThreadData& threadData, NodeData* nodeData, Move bestMove);

    template<NodeType nodeType>
    Score negamax(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats);
    Score quiescenceNegamax(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats);
    void aspirationWindow(ThreadData& threadData, NodeData* rootNode, Score previousScore);

    // Global data
    std::atomic<bool> searchStop;
    TranspositionTable transpositionTable {8 * 1024 * 1024};


    // Thread specific data
    ThreadData data;
    std::thread thread;


};


