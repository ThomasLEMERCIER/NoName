#include "movesorter.hpp"

#include "movegen.hpp"

bool MoveSorter::nextMove(Move& outMove) {
    switch (currentStage) {
        case MoveSorterStage::GeneratingNonQuiets:
            generateMoves<MoveType::NonQuietMoves>(moveList, position);

            currentStage = MoveSorterStage::NonQuiets;
            [[fallthrough]];
        case MoveSorterStage::NonQuiets:
            if (indexMoveList < moveList.getSize()) {
                outMove = moveList[indexMoveList++];
                return true;
            }
            else {
                if (onlyNonQuiets) return false;
                currentStage = MoveSorterStage::GeneratingQuiets;
            }
            [[fallthrough]];
        case MoveSorterStage::GeneratingQuiets:
            generateMoves<MoveType::QuietMoves>(moveList, position);

            currentStage = MoveSorterStage::Quiets;
            [[fallthrough]];
        case MoveSorterStage::Quiets:
            if (indexMoveList < moveList.getSize()) {
                outMove = moveList[indexMoveList++];
                return true;
            }
            else {
                return false;
            }
        default:
            return false;
    }
}
