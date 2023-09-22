#include "transpositiontable.hpp"

void TranspositionTable::initTable(std::uint64_t newMemorySize) {
    std::uint64_t newSize = newMemorySize / sizeof(TTEntry);
    delete[] table;
    table = new TTEntry[newSize];
    size = newSize;
    std::cout << "Transposition Table size: (" << size << ", " << (size * sizeof(TTEntry)) << "B, " << ((size * sizeof(TTEntry)) / (1024. * 1024.)) << "MiB)" << std::endl;
}

void TranspositionTable::writeEntry(const Position &position, std::int16_t depth, ScoreTT score, Move move, Bound bound) {
    if (!table) return;

    TTEntry& entry = table[position.hash % size];
    entry.hash =  position.hash;
    entry.depth = depth;
    entry.bound = bound;
    entry.move = move;
    entry.score = score;
}

bool TranspositionTable::probeTable(std::uint64_t hash, TTEntry &outEntry) {
    if (table) {
        outEntry = table[hash % size];
        if (outEntry.hash == hash) return true;
    }
    return false;
}

ScoreTT TranspositionTable::ScoreToTT(Score score, std::int16_t ply) {
    if (score > checkmateInMaxPly) return (score + ply);
    if (score < -checkmateInMaxPly) return (score - ply);
    return score;

}

Score TranspositionTable::ScoreFromTT(ScoreTT score, std::int16_t ply) {
    if (score > checkmateInMaxPly) return (score - ply);
    if (score < -checkmateInMaxPly) return (score + ply);
    return score;
}

void TranspositionTable::clear() {
    for (std::uint32_t index = 0; index < size; index++) {
        table[index] = {};
    }
}
