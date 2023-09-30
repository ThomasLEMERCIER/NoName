#include "evaluate.hpp"

#include "attacks.hpp"

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
    ScoreExt kingScore = evaluateKing<Color::White>(position) - evaluateKing<Color::Black>(position);

    ScoreExt finalScore = materialScore + pawnScore + pieceScore + kingScore;

    evaluatePhase(evaluationData);
    Score evaluation = interpolateScore(finalScore, evaluationData);

    evaluation = (position.sideToMove == Color::White) ? evaluation : -evaluation;
    return evaluation;
}

void evaluatePhase(EvaluationData& evaluationData) {
    evaluationData.phase = (evaluationData.whiteKnightCount + evaluationData.blackKnightCount) * knightPhaseValue
                         + (evaluationData.whiteBishopCount + evaluationData.blackBishopCount) * bishopPhaseValue
                         + (evaluationData.whiteRookCount + evaluationData.blackRookCount) * rookPhaseValue
                         + (evaluationData.whiteQueenCount + evaluationData.blackQueenCount) * queenPhaseValue;
}

Score interpolateScore(ScoreExt finalScore, EvaluationData& evaluationData) {
    return  (finalScore.mg * evaluationData.phase + finalScore.eg * (phaseMidGame - evaluationData.phase)) / phaseMidGame;
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
    ScoreExt score = (evaluationData.whitePawnCount - evaluationData.blackPawnCount) * pawnValue
                + (evaluationData.whiteKnightCount - evaluationData.blackKnightCount) * knightValue
                + (evaluationData.whiteBishopCount - evaluationData.blackBishopCount) * bishopValue
                + (evaluationData.whiteRookCount - evaluationData.blackRookCount) * rookValue
                + (evaluationData.whiteQueenCount - evaluationData.blackQueenCount) * queenValue;

    return score;
}

template <Color color>
ScoreExt evaluatePawns(const Position &position) {
    constexpr Piece ourPiece = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;
    Bitboard ourPawns = position.getPieces<ourPiece>();

    ScoreExt score{};
    while (ourPawns) {
        Square square = ourPawns.popLsb();

        // PSQT
        score += (color == Color::White) ? pawnSquareTable[square.index()] : pawnSquareTable[square.flipIndex()];
    }

    return score;
}

template<PieceType pieceType, Color color>
ScoreExt evaluatePieces(const Position& position) {
    constexpr Piece piece = getPiece(pieceType, color);
    Bitboard pieceBitboard = position.getPieces<piece>();
    Bitboard occupied = position.occupied;

    ScoreExt score{};
    while (pieceBitboard) {
        Square square = pieceBitboard.popLsb();

        // PSQT
        score += (color == Color::White) ? pieceSquareTable[static_cast<std::uint8_t>(pieceType)][square.index()] : pieceSquareTable[static_cast<std::uint8_t>(pieceType)][square.flipIndex()];

        // Mobility
        if constexpr (pieceType == PieceType::Knight || pieceType == PieceType::Bishop || pieceType == PieceType::Rook || pieceType == PieceType::Queen) {
            Bitboard attacks = getAttacks<pieceType, color>(square, occupied);
            score += mobilityTable[static_cast<std::uint8_t>(pieceType) - 1][attacks.count()];
        }

    }

    return score;
}

template<Color color>
ScoreExt evaluateKing(const Position &position) {
    Bitboard kingBitboard = (color == Color::White) ? position.white.king : position.black.king;
    Square kingSquare = kingBitboard.popLsb();

    // PSQT
    ScoreExt score = (color == Color::White) ? kingSquareTable[kingSquare.index()] : kingSquareTable[kingSquare.flipIndex()];

    return score;
}

bool checkInsufficientMaterial(const Position &position) {
    Bitboard pawnRookQueenBitboard =
            position.white.pawns | position.white.rooks | position.white.queens |
            position.black.pawns | position.black.rooks | position.black.queens;

    if (pawnRookQueenBitboard) return false;

    if (position.white.bishops == 0 && position.black.bishops == 0) {
        // knight & king vs king
        if ((position.white.knights == 0 && position.black.knights.count() < 2) ||
            (position.black.knights == 0 && position.white.knights.count() <2)) return true;
    }

    if (position.white.knights == 0 && position.black.knights == 0) {
        // bishop & king vs king
        if ((position.white.bishops == 0 && position.black.bishops.count() < 2) ||
            (position.black.bishops == 0 && position.white.bishops.count() < 2)) return true;
        // bishop & king vs bishop & king on same color
        if (position.white.bishops.count() == 1 && position.black.bishops.count() == 1) {
            Bitboard lightSquareBishops = (position.white.bishops | position.black.bishops) & Bitboard::LightSquares();
            Bitboard darkSquareBishops = (position.white.bishops | position.black.bishops) & Bitboard::DarkSquares();
            if (darkSquareBishops == 0 || lightSquareBishops == 0) return true;
        }
    }

    return false;
}
