#include "movesorter.hpp"

#include "movegen.hpp"
#include "piece.hpp"
#include "see.hpp"

bool MoveSorter::nextMove(Move& outMove, bool skipQuiet, bool skipBadNonQuiet) {
    std::uint32_t bestIndex = MoveSorter::InvalidIndex;

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

            currentStage = MoveSorterStage::GoodNonQuiets;
            [[fallthrough]];
        case MoveSorterStage::GoodNonQuiets:
            bestIndex = nextSortedIndex(currentIndex, moveList.getSize());

            // no more good non quiets
            if (moveList[bestIndex].score < GoodNonQuietScore) {
                badNonQuietsIndex = currentIndex;
            }
            // good non-quiet
            else {
                outMove = pop(bestIndex);
                return true;
            }

            [[fallthrough]];
        case MoveSorterStage::GeneratingQuiets:
            quietMoveIndex = moveList.getSize();
            if (!skipQuiet) {
                generateMoves<MoveType::QuietMoves>(moveList, position);
                moveList.filter(ttMove);
            }

            [[fallthrough]];
        case MoveSorterStage::Killer1:
            if (!skipQuiet) {
                currentStage = MoveSorterStage::Killer2;
                if (moveList.filter(killerMoves.killer1)) {
                    outMove = killerMoves.killer1;
                    return true;
                }
            }

            [[fallthrough]];
        case MoveSorterStage::Killer2:
            if (!skipQuiet) {
                currentStage = MoveSorterStage::CounterMove;
                if (moveList.filter(killerMoves.killer2)) {
                    outMove = killerMoves.killer2;
                    return true;
                }
            }

            [[fallthrough]];
        case MoveSorterStage::CounterMove:
            if (!skipQuiet) {
                currentStage = MoveSorterStage::OrderingQuiets;
                if (moveList.filter(counterMove)) {
                    outMove = counterMove;
                    return true;
                }
            }

            [[fallthrough]];
        case MoveSorterStage::OrderingQuiets:
            if (!skipQuiet) {
                scoreQuiets();
                currentStage = MoveSorterStage::Quiets;
            }

            [[fallthrough]];
        case MoveSorterStage::Quiets:
            if (!skipQuiet && currentIndex < moveList.getSize()) {
                bestIndex = nextSortedIndex(currentIndex, moveList.getSize());
                outMove = pop(bestIndex);
                return true;
            }
            else {
                currentStage = MoveSorterStage::BadNonQuiets;
                currentIndex = badNonQuietsIndex;
            }
            [[fallthrough]];
        case MoveSorterStage::BadNonQuiets:
            if (!skipBadNonQuiet && currentIndex < quietMoveIndex) {
                bestIndex = nextSortedIndex(currentIndex, quietMoveIndex);
                outMove = pop(bestIndex);
                return true;
            }
            else {
                return false;
            }
    }
    return false;
}

void MoveSorter::scoreNonQuiets() {
    for (std::uint32_t index = currentIndex; index < moveList.getSize(); ++index) {
        PieceType attacker = getPieceType(moveList[index].move.getPiece());
        PieceType victim = getPieceType(position.pieceAt(moveList[index].move.getTo()));

        moveList[index].score = 6 * static_cast<std::int32_t>(victim) - static_cast<std::int32_t>(attacker);

        if (staticExchangeEvaluation(position, moveList[index].move, MoveSorter::GoodNonQuietThreshold)) {
            moveList[index].score += MoveSorter::GoodNonQuietScoreBonus;
        }
    }
}

void MoveSorter::scoreQuiets() {
    for (std::uint32_t index = quietMoveIndex; index < moveList.getSize(); ++index) {
        const Move& move = moveList[index].move;
        moveList[index].score = quietHistoryTable[static_cast<std::uint8_t>(position.sideToMove)][move.getFrom().index()][move.getTo().index()];
    }
}

Move MoveSorter::pop(std::uint32_t index) {
    std::swap<MoveExt>(moveList[index], moveList[currentIndex]);
    return moveList[currentIndex++].move;
}

std::uint32_t MoveSorter::nextSortedIndex(std::uint32_t start, std::uint32_t end) {
    std::uint32_t bestIndex = start;
    for (std::uint32_t index = start + 1; index < end; index++) {
        if (moveList[index].score > moveList[bestIndex].score) bestIndex = index;
    }
    return bestIndex;
}
