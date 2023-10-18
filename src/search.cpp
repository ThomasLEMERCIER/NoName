#include "search.hpp"

#include "evaluate.hpp"

#include <cmath>

std::uint8_t lateMoveReductionTable[64][64];

void Search::startSearch(const Game& game, const SearchLimits& searchLimits) {
    // stop any previous search
    stopSearch();

    Position position = game.getCurrentPosition();

    // Setting thread data
    data.searchLimits = searchLimits;
    data.game = &game;

    data.searchStack[0].position = position;
    data.searchStack[0].inCheck = position.isInCheck(position.sideToMove);

    data.isMainThread = true;
    data.searchStats = {};

    data.moveHistoryTable = {};

    // launching search on thread
    searchStop = false;
    thread = std::thread(&Search::searchInternal, this, std::ref(data));
}

void Search::searchInternal(ThreadData& threadData) {
    NodeData &rootNode = threadData.searchStack[0];
    Move bestMoveSoFar;
    Score previousScore = invalidScore;

    for (std::int16_t currentDepth = 1; currentDepth <= threadData.searchLimits.depthLimit; currentDepth++) {
        rootNode.depth = currentDepth;
        aspirationWindow(threadData, &rootNode, previousScore);

        if (searchStop) break;

        previousScore = rootNode.pvLine.score;
        if (threadData.isMainThread) {
            reportInfo(threadData, &rootNode, threadData.searchStats);
            bestMoveSoFar = rootNode.pvLine.moves[0];
        }
    }

    if (threadData.isMainThread) {
        reportResult(bestMoveSoFar);
    }
}

void Search::aspirationWindow(ThreadData &threadData, NodeData *rootNode, Score previousScore) {
    Score delta = aspirationWindowStart;
    Score alpha = -infValue;
    Score beta  = +infValue;

    if (rootNode->depth >= aspirationWindowMinDepth) {
        alpha = std::min<std::int32_t>(previousScore - delta, +infValue);
        beta  = std::max<std::int32_t>(previousScore + delta, -infValue);
    }

    for (;;) {

        rootNode->alpha = alpha;
        rootNode->beta  = beta;
        rootNode->ply   = 0;

        Score score = negamax<NodeType::Root>(threadData, rootNode, threadData.searchStats);

        if (searchStop) return;

        if (score > alpha && score < beta) {
            rootNode->pvLine.score = score;
            return;
        }

        if (score <= alpha) {
            alpha = alpha - delta;
            beta  = (alpha + beta + 1) / 2;
        }
        else if (score >= beta) {
            beta = beta + delta;
        }

        delta += delta / 2;
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

template<NodeType nodeType>
Score Search::negamax(ThreadData& threadData, NodeData* nodeData, SearchStats& searchStats) {

    if (searchStop || checkStopCondition(threadData.searchLimits, searchStats)) return invalidScore;

    const Position& currentPosition = nodeData->position;
    const Score oldAlpha = nodeData->alpha;
    const std::int16_t depth = nodeData->depth;
    const bool inCheck = nodeData->inCheck;

    constexpr bool rootNode = nodeType == NodeType::Root;
    constexpr bool pvNode = nodeType == NodeType::Root || nodeType == NodeType::Pv;

    PvLine& pvLine = nodeData->pvLine;
    nodeData->pvLine.pvLength = 0;
    searchStats.negamaxNodeCounter++;

    if (!rootNode && (currentPosition.halfMoveCounter >= 100 || checkInsufficientMaterial(currentPosition) || isRepetition(nodeData, threadData.game))) return drawValue;

    if (depth <= 0) return quiescenceNegamax(threadData, nodeData, searchStats);

    if (nodeData->ply >= maxSearchDepth - 1) {
        WARNING("Hit Max Depth search in negamax search, ply count: " << nodeData->ply << "\n")
        return evaluate(currentPosition);
    }

    Score alpha = oldAlpha;
    Score beta = nodeData->beta;

    TTEntry entry;
    Move ttMove = Move::Invalid();
    if (transpositionTable.probeTable(currentPosition.hash, entry)) {
#ifdef SEARCH_STATS
        searchStats.ttHits++;
#endif
        if (!rootNode && entry.depth >= depth) {
            Score ttScore = TranspositionTable::ScoreFromTT(entry.score, nodeData->ply);

            if (entry.bound == Bound::Exact)                     return ttScore;
            if (entry.bound == Bound::Upper && ttScore <= alpha) return ttScore;
            if (entry.bound == Bound::Lower && ttScore >= beta)  return ttScore;
        }
        ttMove = entry.move;
    }

    NodeData& childNode = *(nodeData + 1);
    childNode.clear();
    childNode.ply = nodeData->ply + 1;

    if constexpr (!pvNode) {
        if (!inCheck) {
            Score eval = evaluate(currentPosition);

            if (depth <= reverseFutilityDepth &&
                eval >= beta &&
                eval - futilityMargin(depth) >= beta) {
                return eval;
            }

            if (eval >= beta &&
                depth >= nullMovePruningStartDepth &&
                !nodeData->previousMove.isNull() &&
                currentPosition.hasNonPawnMaterial(currentPosition.sideToMove)) {

                std::int16_t reduction = depth / 4 + nullMovePruningDepthReduction;

                childNode.position = currentPosition;
                childNode.position.doNullMove();
                childNode.previousMove = Move::Null();
                childNode.depth = depth - reduction;
                childNode.inCheck = false;
                childNode.alpha = -beta;
                childNode.beta = -beta + 1;

                Score nullScore = -negamax<NodeType::NonPv>(threadData, &childNode, searchStats);

                if (nullScore >= beta) {
                    return (nullScore >= checkmateInMaxPly) ? beta : nullScore;
                }
            }
        }
    }

    MoveSorter moveSorter {currentPosition, ttMove, threadData.moveHistoryTable};
    std::uint8_t moveCount = 0;
    std::uint8_t quietMoveCount = 0;
    Move outMove;

    Score bestScore = -infValue;
    Move bestMove = Move::Invalid();

    childNode.clear();
    childNode.ply = nodeData->ply + 1;

    bool skipQuiet = false;
    while (moveSorter.nextMove(outMove, skipQuiet)) {
        childNode.position = currentPosition;
        if (!childNode.position.makeMove(outMove))
            continue;

        moveCount++;
        if (outMove.isQuiet()) quietMoveCount++;
        childNode.previousMove = outMove;
        childNode.inCheck = childNode.position.isInCheck(childNode.position.sideToMove);

        if constexpr (!pvNode) {
            if (!inCheck && quietMoveCount >= lateMovePruningThreshold(depth)) {
                skipQuiet = true;
            }
        }

        std::int16_t depthReduction;
        if (outMove.isQuiet()) {
            depthReduction = lateMoveReductionTable[std::min<std::int16_t>(depth, 63)][moveCount];
        }
        else {
            depthReduction = lateMoveReductionTable[std::min<std::int16_t>(depth, 63)][moveCount] / 2;
        }

        Score score;
        if constexpr (pvNode) {
            if (moveCount > 1) {
                if (depthReduction > 0) {
                    childNode.alpha = -(alpha+1);
                    childNode.beta = -alpha;
                    childNode.depth = std::max(depth - depthReduction - 1, 1);

                    score = -negamax<NodeType::NonPv>(threadData, &childNode, searchStats);
                }

                if (depthReduction == 0 || score > alpha) {
                    childNode.alpha = -(alpha + 1);
                    childNode.beta = -alpha;
                    childNode.depth = depth - 1;

                    score = -negamax<NodeType::NonPv>(threadData, &childNode, searchStats);
                }
            }

            if (moveCount == 1 || (score > alpha && (rootNode || score < beta))) {
                childNode.alpha = -beta;
                childNode.beta = -alpha;
                childNode.depth = depth - 1;

                score = -negamax<NodeType::Pv>(threadData, &childNode, searchStats);
            }
        }
        else {
            if (moveCount > 1) {
                if (depthReduction > 0) {
                    childNode.alpha = -(alpha+1);
                    childNode.beta = -alpha;
                    childNode.depth = std::max(depth - depthReduction - 1, 1);

                    score = -negamax<NodeType::NonPv>(threadData, &childNode, searchStats);
                }

                if (depthReduction == 0 || score > alpha) {
                    childNode.alpha = -(alpha + 1);
                    childNode.beta = -alpha;
                    childNode.depth = depth - 1;

                    score = -negamax<NodeType::NonPv>(threadData, &childNode, searchStats);
                }
            }
            else {
                childNode.alpha = -(alpha+1);
                childNode.beta = -alpha;
                childNode.depth = depth - 1;

                score = -negamax<NodeType::NonPv>(threadData, &childNode, searchStats);
            }
        }

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
    if (moveCount == 0) {
        if (currentPosition.isInCheck(currentPosition.sideToMove)) {
            return - (checkmateValue - nodeData->ply);
        }
        return drawValue;
    }

    if (bestScore >= beta) {
        if (bestMove.isQuiet()) {
            updateQuietMoveHistory(threadData, nodeData, bestMove);
        }
    }

    if(!searchStop) {
        Bound bound = (bestScore >= beta) ? Bound::Lower : (bestScore > oldAlpha) ? Bound::Exact : Bound::Upper;
        transpositionTable.writeEntry(currentPosition, depth, TranspositionTable::ScoreToTT(bestScore, nodeData->ply), bestMove, bound);
    }

    return bestScore;
}

Score Search::quiescenceNegamax(ThreadData &threadData, NodeData *nodeData, SearchStats& searchStats) {

    if (searchStop || checkStopCondition(threadData.searchLimits, searchStats)) return invalidScore;

    const Position& currentPosition = nodeData->position;
    const Score oldAlpha = nodeData->alpha;

    searchStats.quiescenceNodeCounter++;

    Score alpha = oldAlpha;
    Score beta = nodeData->beta;

    TTEntry entry;
    Move ttMove = Move::Invalid();
    if (transpositionTable.probeTable(currentPosition.hash, entry)) {
#ifdef SEARCH_STATS
        searchStats.ttHits++;
#endif
        Score ttScore = TranspositionTable::ScoreFromTT(entry.score, nodeData->ply);

        if (entry.bound == Bound::Exact)                     return ttScore;
        if (entry.bound == Bound::Upper && ttScore <= alpha) return ttScore;
        if (entry.bound == Bound::Lower && ttScore >= beta)  return ttScore;

        ttMove = entry.move;
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

    MoveSorter moveSorter {currentPosition, ttMove, threadData.moveHistoryTable};
    Move outMove;
    Move bestMove = Move::Invalid();

    NodeData& childNode = *(nodeData + 1);
    childNode.clear();
    childNode.ply = nodeData->ply + 1;

    while (moveSorter.nextMove(outMove, true)) {
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

constexpr Score Search::futilityMargin(std::int16_t depth) {
    return baseFutilityMargin + scaleFutilityMargin * depth;
}

void Search::updateQuietMoveHistory(ThreadData &threadData, NodeData *nodeData, Move bestMove) {
    // history bonus scaled to be in [0, historyBound (32000)] to go after killer heuristic
    const Color sideToMove = nodeData->position.sideToMove;
    int &history = threadData.moveHistoryTable[static_cast<std::uint8_t>(sideToMove)][bestMove.getFrom().index()][bestMove.getTo().index()];

    std::int32_t bonus = (nodeData->depth * nodeData->depth);
    std::int32_t scaledBonus = bonus - history * bonus / historyBound;
    history += scaledBonus;
}

constexpr std::uint32_t Search::lateMovePruningThreshold(std::int16_t depth) {
    return scaleLateMovePruning * depth + baseLateMovePruning;
}

void initSearchParameters() {
    for (std::uint8_t depth = 1; depth < 64; ++depth) {
        for (std::uint8_t moveIndex = 1; moveIndex < 64; ++moveIndex) {
            lateMoveReductionTable[depth][moveIndex] = std::clamp<std::int32_t>((baseReduction + log(static_cast<double>(depth)) * log(static_cast<double>(moveIndex)) * scaleReduction), 0, 64);
        }
    }

    for (std::uint8_t index = 0; index < 64; ++index) {
        lateMoveReductionTable[0][index] = 0;
        lateMoveReductionTable[1][index] = 0;
        lateMoveReductionTable[index][0] = 0;
    }
}
