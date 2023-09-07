#include "perft.hpp"

#include <iostream>

#include "move.hpp"
#include "movegen.hpp"
#include "movelist.hpp"
#include "utils.hpp"


//#include "movesorter.hpp"

std::uint64_t perftDriver(const Position& pos, const std::uint32_t depth) {
    if (depth == 0)
        return 1;

    std::uint64_t nodes = 0;

//    MoveSorter moveSorter {pos};
//    Move outMove;
//    while (moveSorter.nextMove(outMove)){
//        Position nextPos = pos;
//        if (!nextPos.makeMove(outMove))
//            continue;
//
//        nodes += perftDriver(nextPos, depth - 1);
//    }

    MoveList moveList;
    generateMoves<MoveType::AllMoves>(moveList, pos);
    for (std::uint32_t count = 0; count < moveList.getSize(); ++count) {
        Position nextPos = pos;
        if (!nextPos.makeMove(moveList[count]))
            continue;

        nodes += perftDriver(nextPos, depth - 1);
    }

    return nodes;
}

void perft(const Position& pos, const std::uint32_t depth) {
    std::cout << "Perft to depthLimit " << depth << "\n\n";

    TimePoint startTime = getTime();
    std::uint64_t nodes = 0;


    MoveList moveList;
    generateMoves<MoveType::AllMoves>(moveList, pos);
    for (std::uint32_t count = 0; count < moveList.getSize(); ++count) {
        Position nextPos = pos;
        if (!nextPos.makeMove(moveList[count])) {
            continue;
        }

        std::uint64_t oldNodes = nodes;
        nodes += perftDriver(nextPos, depth - 1);

        std::cout << moveList[count] << ": " << nodes - oldNodes << "\n";
    }

    TimePoint elapsedTime = getTime() - startTime;
    std::uint64_t nps = 1000 * nodes / elapsedTime;
    std::cout << "\n\nNodes: " << nodes << std::endl;
    std::cout << "Time: " << elapsedTime << "ms" << std::endl;
    std::cout << "NPS: " << nps << std::endl;
}
