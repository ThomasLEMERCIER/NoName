#include "movesorter.hpp"

#include "movegen.hpp"
#include "piece.hpp"

bool MoveSorter::nextMove(Move& outMove, bool skipQuiet) {
    switch (currentStage) {
        case MoveSorterStage::TTMove:
            currentStage = MoveSorterStage::GeneratingNonQuiets;
            if (ttMove.isValid() && (!ttMove.isQuiet() || !skipQuiet)) {
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
                if (skipQuiet) return false;
                currentStage = MoveSorterStage::GeneratingQuiets;
            }
            [[fallthrough]];
        case MoveSorterStage::GeneratingQuiets:
            generateMoves<MoveType::QuietMoves>(moveList, position);
            moveList.filter(ttMove);

            currentStage = MoveSorterStage::Killer1;
            [[fallthrough]];
        case MoveSorterStage::Killer1:
            currentStage = MoveSorterStage::Killer2;
            if (moveList.filter(killerMoves.killer1)) {
                outMove = killerMoves.killer1;
                return true;
            }
            [[fallthrough]];
        case MoveSorterStage::Killer2:
            currentStage = MoveSorterStage::OrderingQuiets;
            if (moveList.filter(killerMoves.killer2)) {
                outMove = killerMoves.killer2;
                return true;
            }
            [[fallthrough]];
        case MoveSorterStage::OrderingQuiets:
            scoreQuiets();

            currentStage = MoveSorterStage::Quiets;
            [[fallthrough]];
        case MoveSorterStage::Quiets:
            if (!skipQuiet && indexMoveList < moveList.getSize()) {
                outMove = nextSortedMove();
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
    for (std::uint32_t count = indexMoveList; count < moveList.getSize(); ++count) {
        PieceType attacker = getPieceType(moveList[count].move.getPiece());
        PieceType victim = getPieceType(position.pieceAt(moveList[count].move.getTo()));

        moveList[count].score = 6 * static_cast<std::int32_t>(victim) - static_cast<std::int32_t>(attacker);
    }
}

void MoveSorter::scoreQuiets() {
    for (std::uint32_t count = indexMoveList; count < moveList.getSize(); ++count) {
        const Move& move = moveList[count].move;
        moveList[count].score = quietHistoryTable[static_cast<std::uint8_t>(position.sideToMove)][move.getFrom().index()][move.getTo().index()];
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
