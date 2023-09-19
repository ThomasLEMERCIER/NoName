#pragma once

#include "position.hpp"
#include "utils.hpp"

constexpr Score pawnValue = 100;
constexpr Score knightValue = 300;
constexpr Score bishopValue = 300;
constexpr Score rookValue = 500;
constexpr Score queenValue = 900;

constexpr Score pawnSquareTable[64] = {
          0,   0,   0,   0,   0,   0,   0,   0,
        -13,  -4,   1,   6,   3,  -9,  -9, -16,
        -21, -17,  -1,  12,   8,  -4, -15, -24,
        -14, -21,   9,  10,   4,   4, -20, -17,
        -15, -18, -16,   4,  -2, -18, -23, -17,
        -20,  -9,   1,  17,  36,  -9,  -6, -23,
        -33, -66, -16,  65,  41,  39, -47, -62,
          0,   0,   0,   0,   0,   0,   0,   0,
};

constexpr Score knightSquareTable[64] = {
        -38, -24, -22,  -1,  -1, -19, -20, -30,
         -5,   3, -19,  -2,   0, -16,  -3,   1,
        -21,  -5,   2,  19,  19,   2,  -4, -19,
         21,  30,  41,  50,  53,  41,  28,  26,
         30,  30,  51,  70,  67,  50,  33,  28,
         25,  37,  56,  60,  55,  55,  32,  25,
         -2,  18,  -2,  24,  24,  -7,  16,  -2,
         -5,  12,  41,  17,  19,  48,  24, -17,
};

constexpr Score bishopSquareTable[64] = {
        -21,   1,   5,   5,   8,  -2,   1, -25,
        -17, -31,  -2,   8,   8,  -3, -31, -29,
          3,   9,  -3,  19,  20,  -6,   4,   8,
         12,  17,  32,  32,  34,  30,  17,  14,
         34,  31,  38,  45,  46,  38,  33,  37,
         31,  45,  23,  40,  38,  34,  46,  35,
         38,  22,  30,  36,  36,  33,  21,  35,
         18,  36,  48,  56,  53,  43,  22,  15,
};

constexpr Score rookSquareTable[64] = {
         -1,   3,   4,  -4,  -4,   3,  -2, -14,
          5, -10,  -7, -11, -13, -15, -17,   3,
          3,  14,   9,   2,   3,   8,   9,   1,
         24,  36,  36,  26,  27,  34,  33,  24,
         46,  38,  38,  30,  32,  36,  31,  43,
         60,  41,  54,  36,  35,  52,  32,  56,
         41,  47,  38,  38,  37,  36,  49,  38,
         55,  63,  73,  66,  67,  69,  59,  56,
};

constexpr Score queenSquareTable[64] = {
        -34, -26, -34, -16, -18, -46, -28, -44,
        -15, -22, -42,   2,   0, -49, -29, -18,
         -1,   7,  35,  34,  34,  37,   9, -15,
         17,  46,  59, 109, 106,  57,  48,  33,
         42,  79,  66, 121, 127,  80,  95,  68,
         56,  50,  66,  70,  71,  63,  65,  76,
         61, 108,  65, 114, 120,  59, 116,  73,
         43,  47,  79,  78,  89,  65,  79,  56,
};

constexpr Score kingSquareTable[64] = {
         87,  67,   4,  -9, -10,  -8,  57,  79,
         35, -27, -41, -89, -64, -64, -25,  30,
        -44, -16,  28,   0,  18,  31, -13, -36,
        -48,  98,  71, -22,  12,  79, 115, -59,
         -6,  95,  39, -49, -27,  35,  81, -50,
         24, 123, 105, -22, -39,  74, 100, -17,
          0,  28,   7,  -3, -57,  12,  22, -15,
        -16,  49, -21, -19, -51, -42,  53, -58,
};

constexpr const Score* pieceSquareTable[6] = {
        pawnSquareTable,
        knightSquareTable,
        bishopSquareTable,
        rookSquareTable,
        queenSquareTable,
        kingSquareTable
};

struct EvaluationData {
    std::int16_t whitePawnCount;
    std::int16_t whiteKnightCount;
    std::int16_t whiteBishopCount;
    std::int16_t whiteRookCount;
    std::int16_t whiteQueenCount;

    std::int16_t blackPawnCount;
    std::int16_t blackKnightCount;
    std::int16_t blackBishopCount;
    std::int16_t blackRookCount;
    std::int16_t blackQueenCount;
};

Score evaluate(const Position& position);
void initializeEvaluationData(const Position& position, EvaluationData& evalData);
Score evaluateMaterial(EvaluationData& evaluationData);
template<Color color>
Score evaluatePawns(const Position& position);
template<PieceType pieceType, Color color>
Score evaluatePieces(const Position& position);
template<Color color>
Score evaluateKing(const Position& position);
bool checkInsufficientMaterial(const Position& position);