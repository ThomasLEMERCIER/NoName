#include "movesorter.hpp"

#include "movegen.hpp"
#include "piece.hpp"

bool MoveSorter::nextMove(Move& outMove) {
    switch (currentStage) {
        case MoveSorterStage::TTMove:
            currentStage = MoveSorterStage::GeneratingNonQuiets;
            if (ttMove.isValid() && (!ttMove.isQuiet() || !onlyNonQuiets)) {
                outMove = ttMove;
                return true;
            }
            [[fallthrough]];
        case MoveSorterStage::GeneratingNonQuiets:
            generateMoves<MoveType::NonQuietMoves>(moveList, position);

            moveList.filter(ttMove);
            scoreNonQuiets();

            currentStage = MoveSorterStage::NonQuiets;
            [[fallthrough]];
        case MoveSorterStage::NonQuiets:
            if (indexMoveList < moveList.getSize()) {
                outMove = nextSortedMove();
                return true;
            }
            else {
                if (onlyNonQuiets) return false;
                currentStage = MoveSorterStage::GeneratingQuiets;
            }
            [[fallthrough]];
        case MoveSorterStage::GeneratingQuiets:
            generateMoves<MoveType::QuietMoves>(moveList, position);

            moveList.filter(ttMove);

            currentStage = MoveSorterStage::Quiets;
            [[fallthrough]];
        case MoveSorterStage::Quiets:
            if (indexMoveList < moveList.getSize()) {
                outMove = moveList[indexMoveList++].move;
                return true;
            }
            else {
                return false;
            }
        default:
            return false;
    }
}

void MoveSorter::scoreNonQuiets() {
    for (std::uint32_t count = 0; count < moveList.getSize(); ++count) {
        PieceType attacker = getPieceType(moveList[count].move.getPiece());
        PieceType victim = getPieceType(position.pieceAt(moveList[count].move.getTo()));

        moveList[count].score = 6 * static_cast<std::int32_t>(victim) - static_cast<std::int32_t>(attacker);
    }
}

Move MoveSorter::nextSortedMove() {
    std::uint32_t bestIndex = indexMoveList;
    for (std::uint32_t index = indexMoveList + 1; index < moveList.getSize(); index++) {
        if (moveList[index].score > moveList[bestIndex].score) bestIndex = index;
    }

    MoveExt temp = moveList[indexMoveList];
    moveList[indexMoveList] = moveList[bestIndex];
    moveList[bestIndex] = temp;
    return moveList[indexMoveList++].move;
}
