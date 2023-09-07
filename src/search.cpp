#include "search.hpp"

#include "evaluate.hpp"
#include "movesorter.hpp"

void Search::startSearch(const Position& position, const SearchLimits& searchLimits) {
    // stop any previous search
    stopSearch();

    // Setting thread data
    data.searchLimits = searchLimits;
    data.isMainThread = true;
    data.searchStats = {};
    data.searchStack[0].position = position;

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
    std::cout << "\nStats: BetaCutoff " << searchStats.betaCutoff;
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

    searchStats.negamaxNodeCounter++;

    if (nodeData->depth <= 0) return quiescenceNegamax(threadData, nodeData, searchStats);

    if (nodeData->ply >= maxSearchDepth - 1) {
        WARNING("Hit Max Depth search in negamax search, ply count: " << nodeData->ply << "\n")
        return evaluate(currentPosition);
    }

    Score bestScore = -infValue;
    Score alpha = oldAlpha;
    Score beta = nodeData->beta;

    MoveSorter moveSorter {currentPosition};
    std::uint8_t legalMoveCounter = 0;
    Move outMove;

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

        Score score = -negamax(threadData, &childNode, searchStats);

        if (score > bestScore) {
            bestScore = score;
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

    return bestScore;
}

Score Search::quiescenceNegamax(ThreadData &threadData, NodeData *nodeData, SearchStats& searchStats) {

    if (searchStop || checkStopCondition(threadData.searchLimits, searchStats)) return invalidScore;

    Position& currentPosition = nodeData->position;
    Score oldAlpha = nodeData->alpha;

    searchStats.quiescenceNodeCounter++;

    Score staticEvaluation = evaluate(currentPosition);

    if (nodeData->ply >= maxSearchDepth - 1) {
        WARNING("Hit Max Depth search in Quiescence search, ply count: " << nodeData->ply << "\n")
        return staticEvaluation;
    }

    Score alpha = oldAlpha;
    Score beta = nodeData->beta;
    Score bestScore = staticEvaluation;

    if (bestScore >= beta)
        return staticEvaluation;

    if (alpha < bestScore)
        alpha = bestScore;

    MoveSorter moveSorter {currentPosition, true};
    Move outMove;

    NodeData& childNode = *(nodeData + 1);
    childNode.clear();
    childNode.ply = nodeData->ply + 1;

    while (moveSorter.nextMove(outMove)) {
        childNode.position = currentPosition;
        if (!childNode.position.makeMove(outMove))
            continue;

        childNode.alpha = -beta;
        childNode.beta = -alpha;
        Score score = -quiescenceNegamax(threadData, &childNode, searchStats);

        if (score > bestScore) {
            bestScore = score;
        }

        if (score > alpha) {
            alpha = score;

            if (score >= beta) {
                break;
            }
        }
    }

    return bestScore;
}

