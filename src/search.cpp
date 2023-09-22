#include "search.hpp"

#include "evaluate.hpp"
#include "movesorter.hpp"

void Search::startSearch(const Game& game, const SearchLimits& searchLimits) {
    // stop any previous search
    stopSearch();

    // Setting thread data
    data.searchLimits = searchLimits;
    data.game = &game;
    data.isMainThread = true;
    data.searchStats = {};
    data.searchStack[0].position = game.getCurrentPosition();

    // launching search on thread
    searchStop = false;
    thread = std::thread(&Search::searchInternal, this, std::ref(data));
}

void Search::searchInternal(ThreadData& threadData) {
    NodeData &rootNode = threadData.searchStack[0];
    Move bestMoveSoFar;

    for (std::uint8_t currentDepth = 1; currentDepth <= threadData.searchLimits.depthLimit; currentDepth++) {
        rootNode.alpha = -infValue;
        rootNode.beta = +infValue;
        rootNode.depth = currentDepth;
        rootNode.ply = 0;

        rootNode.pvLine.score = negamax(threadData, &rootNode, threadData.searchStats);
        if (searchStop) break;

        if (threadData.isMainThread) {
            reportInfo(threadData, &rootNode, threadData.searchStats);
            bestMoveSoFar = rootNode.pvLine.moves[0];
        }
    }

    if (threadData.isMainThread) {
        reportResult(bestMoveSoFar);
    }
}

void Search::stopSearch() {
    searchStop = true;
    if (thread.joinable()) thread.join();
}

bool Search::checkStopCondition(SearchLimits& searchLimits, SearchStats& searchStats){
    // check time limits every 2048 nodes if needed
    if (searchLimits.timeLimit != invalidTimePoint && ((searchStats.negamaxNodeCounter + searchStats.quiescenceNodeCounter) % 2048) == 0) {
        if (getTime() >= searchLimits.timeLimit) {
            searchStop = true;
            return true;
        }
    }
    return false;
}

void Search::reportInfo(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats) {
    std::uint64_t totalNodes = searchStats.negamaxNodeCounter + searchStats.quiescenceNodeCounter;
    TimePoint searchTime = (getTime() - threadData.searchLimits.searchTimeStart + 1);
    std::uint32_t nps = totalNodes / searchTime * 1000;
    Score score = nodeData->pvLine.score;

    std::cout << "info depth " << nodeData->depth;
    std::cout << " nodes " << totalNodes;
    std::cout << " time " << searchTime << "ms";
    std::cout << " nps " << nps;

    if (score > checkmateInMaxPly)
        std::cout << " score mate " << (checkmateValue - score);
    else if (score < - checkmateInMaxPly)
        std::cout << " score mate " << -(checkmateValue + score);
    else
        std::cout << " score cp " << score;

    std::cout << " pv ";
    for (std::uint32_t i = 0; i < nodeData->pvLine.pvLength; i++) {
        std::cout << nodeData->pvLine.moves[i] << " ";
    }

#ifdef SEARCH_STATS
    std::cout << "\nStats: NegamaxNodes: " << searchStats.negamaxNodeCounter;
    std::cout << "\nStats: QuiescenceNodes: " << searchStats.quiescenceNodeCounter;
    std::cout << "\nStats: BetaCutoff: " << searchStats.betaCutoff;
    std::cout << "\nStats: TTHits: " << searchStats.ttHits;
#endif

    std::cout << std::endl;
}

void Search::reportResult(Move bestMove) {
    std::cout << "bestmove " << bestMove << std::endl;
}

Score Search::negamax(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats) {

    if (searchStop || checkStopCondition(threadData.searchLimits, searchStats)) return invalidScore;

    Position& currentPosition = nodeData->position;
    PvLine& pvLine = nodeData->pvLine;
    Score oldAlpha = nodeData->alpha;
    bool rootNode = nodeData->ply == 0;

    nodeData->pvLine.pvLength = 0;
    searchStats.negamaxNodeCounter++;

    if (!rootNode && (currentPosition.halfMoveCounter >= 100 || checkInsufficientMaterial(currentPosition) || isRepetition(nodeData, threadData.game))) return drawValue;

    if (nodeData->depth <= 0) return quiescenceNegamax(threadData, nodeData, searchStats);

    if (nodeData->ply >= maxSearchDepth - 1) {
        WARNING("Hit Max Depth search in negamax search, ply count: " << nodeData->ply << "\n")
        return evaluate(currentPosition);
    }

    Score bestScore = -infValue;
    Score alpha = oldAlpha;
    Score beta = nodeData->beta;

    TTEntry entry;
    if (transpositionTable.probeTable(currentPosition.hash, entry)) {
#ifdef SEARCH_STATS
        searchStats.ttHits++;
#endif
        if (!rootNode && entry.depth >= nodeData->depth) {
            Score ttScore = TranspositionTable::ScoreFromTT(entry.score, nodeData->ply);

            if (entry.bound == Bound::Exact)                     return ttScore;
            if (entry.bound == Bound::Upper && ttScore <= alpha) return ttScore;
            if (entry.bound == Bound::Lower && ttScore >= beta)  return ttScore;
        }
    }

    MoveSorter moveSorter {currentPosition};
    std::uint8_t legalMoveCounter = 0;
    Move outMove;
    Move bestMove = Move::Invalid();

    NodeData& childNode = *(nodeData + 1);
    childNode.clear();
    childNode.ply = nodeData->ply + 1;

    std::int16_t depthReduction = 1;

    while (moveSorter.nextMove(outMove)) {
        childNode.position = currentPosition;
        if (!childNode.position.makeMove(outMove))
            continue;

        legalMoveCounter++;

        childNode.alpha = -beta;
        childNode.beta = -alpha;
        childNode.depth = nodeData->depth - depthReduction;
        childNode.previousMove = outMove;

        Score score = -negamax(threadData, &childNode, searchStats);

        if (score > bestScore) {
            bestScore = score;
            bestMove = outMove;
        }

        if (score > alpha) {
            alpha = score;

            pvLine.moves[0] = outMove;
            for (std::uint8_t i = 0; i < childNode.pvLine.pvLength; i++)
                pvLine.moves[i + 1] = childNode.pvLine.moves[i];
            pvLine.pvLength = childNode.pvLine.pvLength + 1;

            if (score >= beta) {
#ifdef SEARCH_STATS
                searchStats.betaCutoff++;
#endif
                break;
            }
        }
    }

    // either checkmate or stalemate
    if (legalMoveCounter == 0) {
        if (currentPosition.isInCheck(currentPosition.sideToMove)) {
            return - (checkmateValue - nodeData->ply);
        }
        return drawValue;
    }

    if(!searchStop) {
        Bound bound = (bestScore >= beta) ? Bound::Lower : (bestScore > oldAlpha) ? Bound::Exact : Bound::Upper;
        transpositionTable.writeEntry(currentPosition, nodeData->depth, TranspositionTable::ScoreToTT(bestScore, nodeData->ply), bestMove, bound);
    }

    return bestScore;
}

Score Search::quiescenceNegamax(ThreadData &threadData, NodeData *nodeData, SearchStats& searchStats) {

    if (searchStop || checkStopCondition(threadData.searchLimits, searchStats)) return invalidScore;

    Position& currentPosition = nodeData->position;
    Score oldAlpha = nodeData->alpha;

    searchStats.quiescenceNodeCounter++;

    Score alpha = oldAlpha;
    Score beta = nodeData->beta;

    TTEntry entry;
    if (transpositionTable.probeTable(currentPosition.hash, entry)) {
#ifdef SEARCH_STATS
        searchStats.ttHits++;
#endif
        Score ttScore = TranspositionTable::ScoreFromTT(entry.score, nodeData->ply);

        if (entry.bound == Bound::Exact)                     return ttScore;
        if (entry.bound == Bound::Upper && ttScore <= alpha) return ttScore;
        if (entry.bound == Bound::Lower && ttScore >= beta)  return ttScore;
    }

    Score staticEvaluation = evaluate(currentPosition);
    Score bestScore = staticEvaluation;

    if (nodeData->ply >= maxSearchDepth - 1) {
        WARNING("Hit Max Depth search in Quiescence search, ply count: " << nodeData->ply << "\n")
        return staticEvaluation;
    }

    if (bestScore >= beta)
        return staticEvaluation;

    if (alpha < bestScore)
        alpha = bestScore;

    MoveSorter moveSorter {currentPosition, true};
    Move outMove;
    Move bestMove = Move::Invalid();

    NodeData& childNode = *(nodeData + 1);
    childNode.clear();
    childNode.ply = nodeData->ply + 1;

    while (moveSorter.nextMove(outMove)) {
        childNode.position = currentPosition;
        if (!childNode.position.makeMove(outMove))
            continue;

        childNode.alpha = -beta;
        childNode.beta = -alpha;
        childNode.previousMove = outMove;

        Score score = -quiescenceNegamax(threadData, &childNode, searchStats);

        if (score > bestScore) {
            bestScore = score;
            bestMove = outMove;
        }

        if (score > alpha) {
            alpha = score;

            if (score >= beta) {
#ifdef SEARCH_STATS
                searchStats.betaCutoff++;
#endif
                break;
            }
        }
    }

    if(!searchStop) {
        Bound bound = (bestScore >= beta) ? Bound::Lower : (bestScore > oldAlpha) ? Bound::Exact : Bound::Upper;
        transpositionTable.writeEntry(currentPosition, 0, TranspositionTable::ScoreToTT(bestScore, nodeData->ply), bestMove, bound);
    }

    return bestScore;
}

bool Search::isRepetition(NodeData* nodeData, const Game* game) {
    NodeData* previousNode = nodeData;
    std::uint32_t plyCounter = 0;

    const std::uint64_t targetHash = nodeData->position.hash;

    // Check for repetition inside the current search
    while (previousNode->ply > 0) {
        // Irreversible moves
        const Move& prevMove = previousNode->previousMove;
        if (prevMove.isCapture() || (prevMove.getPiece() == getPiece(PieceType::Pawn, ~previousNode->position.sideToMove))) {
            return false;
        }

        --previousNode;
        ++plyCounter;

        if (plyCounter % 2 == 0) {
            if (previousNode->position.hash == targetHash) {
                return true;
            }
        }
    }

    // Check for repetition outside the current search
    return game->checkRepetition(targetHash);
}
