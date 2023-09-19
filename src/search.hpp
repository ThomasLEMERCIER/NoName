#pragma once

#include "game.hpp"
#include "move.hpp"
#include "position.hpp"
#include "utils.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <thread>

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

    Score alpha;
    Score beta;

    std::int16_t depth;
    std::int16_t ply;

    PvLine pvLine;
    Move previousMove;

    void clear() {
        position = {};
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
#endif
};

struct ThreadData {
    SearchLimits searchLimits;
    const Game* game;

    std::array<NodeData, maxSearchDepth> searchStack;

    bool isMainThread;
    SearchStats searchStats;
};

class Search {
public:
    void startSearch(const Game& game, const SearchLimits& searchLimits);
    void stopSearch();
    void searchInternal(ThreadData& threadData);

private:
    static void reportInfo(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats);
    static void reportResult(Move bestMove);
    bool checkStopCondition(SearchLimits& searchLimits, SearchStats& searchStats);
    static bool isRepetition(NodeData* nodeData, const Game* game);

    Score negamax(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats);
    Score quiescenceNegamax(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats);

    // Global data
    std::atomic<bool> searchStop;

    // Thread specific data
    ThreadData data;
    std::thread thread;

};


