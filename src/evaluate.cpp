#include "evaluate.hpp"

Score evaluate(const Position &position) {
    std::uint8_t whitePawnCount = position.white.pawns.count();
    std::uint8_t whiteKnightCount = position.white.knights.count();
    std::uint8_t whiteBishopCount = position.white.bishops.count();
    std::uint8_t whiteRookCount = position.white.rooks.count();
    std::uint8_t whiteQueenCount = position.white.queens.count();

    std::uint8_t blackPawnCount = position.black.pawns.count();
    std::uint8_t blackKnightCount = position.black.knights.count();
    std::uint8_t blackBishopCount = position.black.bishops.count();
    std::uint8_t blackRookCount = position.black.rooks.count();
    std::uint8_t blackQueenCount = position.black.queens.count();

    Score materialScore = whitePawnCount * pawnValue;
    materialScore += whiteKnightCount * knightValue;
    materialScore += whiteBishopCount * bishopValue;
    materialScore += whiteRookCount * rookValue;
    materialScore += whiteQueenCount * queenValue;

    materialScore -= blackPawnCount * pawnValue;
    materialScore -= blackKnightCount * knightValue;
    materialScore -= blackBishopCount * bishopValue;
    materialScore -= blackRookCount * rookValue;
    materialScore -= blackQueenCount * queenValue;

    Score finalScore = (position.sideToMove == Color::White) ? (materialScore) : -(materialScore);
    return finalScore;
}
