#include "evaluate.hpp"

Score evaluate(const Position &position) {
    EvaluationData evaluationData{};
    initializeEvaluationData(position, evaluationData);

    Score materialScore = evaluateMaterial(evaluationData);
    Score pawnScore = evaluatePawns<Color::White>(position) - evaluatePawns<Color::Black>(position);
    Score pieceScore =
            evaluatePieces<PieceType::Knight, Color::White>(position) - evaluatePieces<PieceType::Knight, Color::Black>(position) +
            evaluatePieces<PieceType::Bishop, Color::White>(position) - evaluatePieces<PieceType::Bishop, Color::Black>(position) +
            evaluatePieces<PieceType::Rook, Color::White>(position) - evaluatePieces<PieceType::Rook, Color::Black>(position) +
            evaluatePieces<PieceType::Queen, Color::White>(position) - evaluatePieces<PieceType::Queen, Color::Black>(position);
    Score kingScore = evaluateKing<Color::White>(position) - evaluateKing<Color::Black>(position);

    Score finalScore = materialScore + pawnScore + pieceScore + kingScore;

    finalScore = (position.sideToMove == Color::White) ? finalScore : -finalScore;
    return finalScore;
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

Score evaluateMaterial(EvaluationData& evaluationData) {
    Score score = (evaluationData.whitePawnCount - evaluationData.blackPawnCount) * pawnValue
                + (evaluationData.whiteKnightCount - evaluationData.blackKnightCount) * knightValue
                + (evaluationData.whiteBishopCount - evaluationData.blackBishopCount) * bishopValue
                + (evaluationData.whiteRookCount - evaluationData.blackRookCount) * rookValue
                + (evaluationData.whiteQueenCount - evaluationData.blackQueenCount) * queenValue;

    return score;
}

template <Color color>
Score evaluatePawns(const Position &position) {
    constexpr Piece ourPiece = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;
    Bitboard ourPawns = position.getPieces<ourPiece>();

    Score score{};
    while (ourPawns) {
        Square square = ourPawns.popLsb();

        // PSQT
        score += (color == Color::White) ? pawnSquareTable[square.index()] : pawnSquareTable[square.flipIndex()];
    }

    return score;
}

template<PieceType pieceType, Color color>
Score evaluatePieces(const Position& position) {
    constexpr Piece piece = getPiece(pieceType, color);
    Bitboard pieceBitboard = position.getPieces<piece>();

    Score score{};
    while (pieceBitboard) {
        Square square = pieceBitboard.popLsb();

        // PSQT
        score += (color == Color::White) ? pieceSquareTable[static_cast<std::uint8_t>(pieceType)][square.index()] : pieceSquareTable[static_cast<std::uint8_t>(pieceType)][square.flipIndex()];
    }

    return score;
}

template<Color color>
Score evaluateKing(const Position &position) {
    Bitboard kingBitboard = (color == Color::White) ? position.white.king : position.black.king;
    Square kingSquare = kingBitboard.popLsb();

    // PSQT
    Score score = (color == Color::White) ? kingSquareTable[kingSquare.index()] : kingSquareTable[kingSquare.flipIndex()];

    return score;
}
