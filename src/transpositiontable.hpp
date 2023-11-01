#pragma once

#include "move.hpp"
#include "position.hpp"
#include "utils.hpp"

#include <cstdint>

enum class Bound: std::uint8_t {
    Lower,
    Upper,
    Exact
};

struct TTEntry {
    std::uint64_t hash;
    std::int16_t depth;

    Bound bound;
    ScoreTT score;
    Move move;
};

class TranspositionTable {
public:
    explicit TranspositionTable(std::uint64_t initSize) : table{nullptr}, size{0} { initTable(initSize); };
    ~TranspositionTable() { delete[] table; };

    void initTable(std::uint64_t newMemorySize);
    void writeEntry(const Position& position, std::int16_t depth, ScoreTT score, Move move, Bound bound);
    void prefetchTable(std::uint64_t hash);
    bool probeTable(std::uint64_t hash, TTEntry& outEntry);
    void clear();

    static ScoreTT ScoreToTT(Score score, std::int16_t ply);
    static Score ScoreFromTT(ScoreTT score, std::int16_t ply);
private:
    TTEntry* table;
    std::uint64_t size;
};
