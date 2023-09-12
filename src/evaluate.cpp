#include "evaluate.hpp"

#include "attacks.hpp"

Bitboard passedPawnSpans[2][64];
Bitboard isolatedPawnSpans[64];

Score evaluate(const Position &position) {
    EvaluationData evaluationData{};
    initializeEvaluationData(position, evaluationData);

    ScoreExt materialScore = evaluateMaterial(evaluationData);
    ScoreExt pawnScore = evaluatePawns<Color::White>(position) - evaluatePawns<Color::Black>(position);
    ScoreExt pieceScore =
            evaluatePieces<PieceType::Knight, Color::White>(position) - evaluatePieces<PieceType::Knight, Color::Black>(position) +
            evaluatePieces<PieceType::Bishop, Color::White>(position) - evaluatePieces<PieceType::Bishop, Color::Black>(position) +
            evaluatePieces<PieceType::Rook, Color::White>(position) - evaluatePieces<PieceType::Rook, Color::Black>(position) +
            evaluatePieces<PieceType::Queen, Color::White>(position) - evaluatePieces<PieceType::Queen, Color::Black>(position);
    ScoreExt kingScore =
            evaluateKing<Color::White>(position) - evaluateKing<Color::Black>(position);

    ScoreExt finalScore = materialScore + pawnScore + pieceScore + kingScore;

    evaluatePhase(evaluationData);
    Score evaluation = interpolateScore(finalScore, evaluationData);

    evaluation += (position.sideToMove == Color::White) ? 2 : -2;
    evaluation = (position.sideToMove == Color::White) ? evaluation : -evaluation;
    return evaluation;
}

void initializeEvaluationData(const Position& position, EvaluationData& evaluationData) {
    evaluationData.whitePawnCount = position.white.pawns.count();
    evaluationData.whiteKnightCount = position.white.knights.count();
    evaluationData.whiteBishopCount = position.white.bishops.count();
    evaluationData.whiteRookCount = position.white.rooks.count();
    evaluationData.whiteQueenCount = position.white.queens.count();

    evaluationData.blackPawnCount = position.black.pawns.count();
    evaluationData.blackKnightCount = position.black.knights.count();
    evaluationData.blackBishopCount = position.black.bishops.count();
    evaluationData.blackRookCount = position.black.rooks.count();
    evaluationData.blackQueenCount = position.black.queens.count();
}

ScoreExt evaluateMaterial(EvaluationData& evaluationData) {
    ScoreExt score = evaluationData.whitePawnCount * pawnValue;
    score += evaluationData.whiteKnightCount * knightValue;
    score += evaluationData.whiteBishopCount * bishopValue;
    score += evaluationData.whiteRookCount * rookValue;
    score += evaluationData.whiteQueenCount * queenValue;

    score -= evaluationData.blackPawnCount * pawnValue;
    score -= evaluationData.blackKnightCount * knightValue;
    score -= evaluationData.blackBishopCount * bishopValue;
    score -= evaluationData.blackRookCount * rookValue;
    score -= evaluationData.blackQueenCount * queenValue;
    return score;
}

void evaluatePhase(EvaluationData& evaluationData) {
    evaluationData.phase = (evaluationData.whiteKnightCount + evaluationData.blackKnightCount) * knightPhaseValue
                         + (evaluationData.whiteBishopCount + evaluationData.blackBishopCount) * bishopPhaseValue
                         + (evaluationData.whiteRookCount + evaluationData.blackRookCount) * rookPhaseValue
                         + (evaluationData.whiteQueenCount + evaluationData.blackQueenCount) * queenPhaseValue;
}

template <Color color>
ScoreExt evaluatePawns(const Position &position) {
    constexpr Piece ourPiece = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;
    constexpr Piece theirPiece = (color == Color::White) ? Piece::BlackPawn : Piece::WhitePawn;

    Bitboard ourPawns = position.getPieces<ourPiece>();
    Bitboard theirPawns = position.getPieces<theirPiece>();

    ScoreExt score{};
    while (ourPawns) {
        Square square = ourPawns.popLsb();

        // PSQT
        score += (color == Color::White) ? pieceSquareTable[0][square.index()] : pieceSquareTable[0][square.flipIndex()];

        // Pawns structure evaluation
        Bitboard doublePawn = ourPawns & Bitboard::FileBitboard(square.file());
        Bitboard passedPawn = passedPawnSpans[static_cast<std::uint8_t>(color)][square.index()] & theirPawns;
        Bitboard isolatedPawn = isolatedPawnSpans[square.index()] & ourPawns;

        if (doublePawn) {
            score += doublePawnPenalty;
        }

        if (passedPawn == 0ULL) {
            score += passedPawnBonus;
        }

        if (isolatedPawn == 0ULL) {
            score += isolatedPawnPenalty;
        }
    }

    return score;
}

Score interpolateScore(ScoreExt finalScore, EvaluationData& evaluationData) {
    return  (finalScore.mg * evaluationData.phase + finalScore.eg * (phaseMidGame - evaluationData.phase)) / phaseMidGame;
}

template<PieceType pieceType, Color color>
ScoreExt evaluatePieces(const Position& position) {
    constexpr Piece piece = getPiece(pieceType, color);
    Bitboard pieceBitboard = position.getPieces<piece>();

    ScoreExt score{};
    while (pieceBitboard) {
        Square square = pieceBitboard.popLsb();

        // PSQT
        score += (color == Color::White) ? pieceSquareTable[static_cast<std::uint8_t>(pieceType)][square.index()] : pieceSquareTable[static_cast<std::uint8_t>(pieceType)][square.flipIndex()];

        // Mobility
        if constexpr (pieceType == PieceType::Knight || pieceType == PieceType::Bishop || pieceType == PieceType::Rook || pieceType == PieceType::Queen){
            Bitboard attacks = getAttacks<pieceType, color>(square, position.occupied);
            score += mobilityBonus[static_cast<std::uint8_t>(pieceType) - 1][attacks.count()];
        }

        if constexpr (pieceType == PieceType::Rook) {
            // open file bonus
            if (!((position.white.pawns | position.black.pawns) & Bitboard::FileBitboard(square.file()))) {
                score += rookOpenFileBonus;
            }
        }
    }

    return score;
}

template<Color color>
ScoreExt evaluateKing(const Position &position) {
    constexpr Direction forward = (color == Color::White) ? Direction::North : Direction::South;
    Bitboard kingBitboard = (color == Color::White) ? position.white.king : position.black.king;
    Bitboard ourPawns = (color == Color::White) ? position.white.pawns : position.black.pawns;
    Square kingSquare = kingBitboard.popLsb();

    ScoreExt score{};

    // PSQT
    score += (color == Color::White) ? pieceSquareTable[5][kingSquare.index()] : pieceSquareTable[5][kingSquare.flipIndex()];

    // Pawn Shield
    Bitboard shieldSpan = kingBitboard.shift<forward>();
    shieldSpan |= shieldSpan.east() | shieldSpan.west();
    shieldSpan &= ourPawns;
    score += pawnShieldBonus[shieldSpan.count()];

    return score;
}

void initEvaluation() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square {sq};

        Bitboard fileBitboard = Bitboard::FileBitboard(square.file());
        isolatedPawnSpans[sq] = fileBitboard.west() | fileBitboard.east();

        Bitboard adjacent {square};
        adjacent |= adjacent.west() | adjacent.east();

        Bitboard whiteNextRank = adjacent.north();
        Bitboard whitePassedPawnSpan{};
        while (whiteNextRank) {
            whitePassedPawnSpan |= whiteNextRank;
            whiteNextRank = whiteNextRank.shift<Direction::North>();
        }

        Bitboard blackNextRank = adjacent.south();
        Bitboard blackPassedPawnSpan{};
        while (blackNextRank) {
            blackPassedPawnSpan |= blackNextRank;
            blackNextRank = blackNextRank.shift<Direction::South>();
        }

        passedPawnSpans[0][sq] = whitePassedPawnSpan;
        passedPawnSpans[1][sq] = blackPassedPawnSpan;
    }
}
