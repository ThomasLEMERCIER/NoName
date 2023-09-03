#include "perft.hpp"

#include <iostream>

#include "move.hpp"
#include "movegen.hpp"
#include "movelist.hpp"
#include "timeman.hpp"

std::uint64_t perftDriver(const Position& pos, const std::uint32_t depth) {
    if (depth == 0)
        return 1;

    MoveList moveList;
    generateMoves<MoveType::AllMoves>(moveList, pos);

    std::uint64_t nodes = 0;
    for (std::uint32_t count = 0; count < moveList.getSize(); ++count) {
        auto nextPos = Position(pos);
        if (!nextPos.makeMove(moveList[count]))
            continue;

        nodes += perftDriver(nextPos, depth - 1);
    }

    return nodes;
}

void perft(const Position& pos, const std::uint32_t depth) {
    std::cout << "Perft to depth " << depth << "\n\n";

    TimePoint startTime = getTime();
    MoveList moveList;
    generateMoves<MoveType::AllMoves>(moveList, pos);
    
    std::uint64_t nodes = 0;

    for (std::uint32_t count = 0; count < moveList.getSize(); ++count) {
        auto nextPos = Position(pos);
        if (!nextPos.makeMove(moveList[count])) {
            continue;
        }
        

        std::uint64_t oldNodes = nodes;

        nodes += perftDriver(nextPos, depth - 1);

        std::cout << moveList[count] << ": " << nodes - oldNodes << "\n";
    }

    TimePoint endTime = getTime();
    std::cout << "\n\nNodes: " << nodes << std::endl;
    std::cout << "Time: " << endTime - startTime << "ms" << std::endl;
    std::cout << "NPS: " << static_cast<std::uint32_t>(nodes / ((endTime - startTime) / 1000.0)) << std::endl;
}
