#pragma once

#include "position.hpp"
#include "utils.hpp"

struct ScoreExt {
    Score mg;
    Score eg;
};

#define S(a, b) ScoreExt{a, b}

constexpr ScoreExt operator+(const ScoreExt& s1, const ScoreExt& s2) { return { static_cast<Score>(s1.mg + s2.mg), static_cast<Score>(s1.eg + s2.eg) }; }
constexpr ScoreExt operator-(const ScoreExt& s1, const ScoreExt& s2) { return { static_cast<Score>(s1.mg - s2.mg), static_cast<Score>(s1.eg - s2.eg) }; }
constexpr ScoreExt operator+=(ScoreExt& s1, const ScoreExt& s2) { return s1 = s1 + s2; }
constexpr ScoreExt operator-=(ScoreExt& s1, const ScoreExt& s2) { return s1 = s1 - s2; }

constexpr ScoreExt operator*(std::int32_t lhs, ScoreExt rhs) { return {static_cast<Score>(rhs.mg * lhs), static_cast<Score>(rhs.eg * lhs) };};

constexpr ScoreExt pawnValue = {82, 144};
constexpr ScoreExt knightValue = {426, 475};
constexpr ScoreExt bishopValue = {441, 510};
constexpr ScoreExt rookValue = {627, 803};
constexpr ScoreExt queenValue = {1292, 1623};

constexpr ScoreExt mobilityBonus[][32] = {
        { S(-104,-139), S( -45,-114), S( -22, -37), S(  -8,   3), S(   6,  15), S(  11,  34), S(  19,  38), S(  30,  37), S(  43,  17) }, // Knight

        { S( -99,-186), S( -46,-124), S( -16, -54), S(  -4, -14), S(   6,   1), S(  14,  20), S(  17,  35), S(  19,  39), S(  19,  49),   // Bishop
                S(  27,  48), S(  26,  48), S(  52,  32), S(  55,  47), S(  83,   2) },

        { S(-127,-148), S( -56,-127), S( -25, -85), S( -12, -28), S( -10,   2), S( -12,  27), S( -11,  42), S(  -4,  46), S(   4,  52),   // Rook
                S(   9,  55), S(  11,  64), S(  19,  68), S(  19,  73), S(  37,  60), S(  97,  15) },

        { S(-111,-273), S(-253,-401), S(-127,-228), S( -46,-236), S( -20,-173), S(  -9, -86), S(  -1, -35), S(   2,  -1), S(   8,   8),   // Queen
                S(  10,  31), S(  15,  37), S(  17,  55), S(  20,  46), S(  23,  57), S(  22,  58), S(  21,  64), S(  24,  62), S(  16,  65),
                S(  13,  63), S(  18,  48), S(  25,  30), S(  38,   8), S(  34, -12), S(  28, -29), S(  10, -44), S(   7, -79), S( -42, -30),
                S( -23, -50) }
};

constexpr ScoreExt passedPawnBonus[8]   = { S(   0,   0), S( -28,  23), S( -40,  35), S( -55,  60), S(   8,  89), S(  95, 166), S( 124, 293), S(   0,   0) }; // per rank
constexpr ScoreExt isolatedPawnBonus[8] = { S(-13, -12), S(-1, -16), S(1, -16), S(3, -18), S(7, -19), S(3, -15), S(-4, -14), S(-4, -17) }; // per file
constexpr ScoreExt doubledPawnBonus[8]  = { S(10, -29), S(-2, -26), S(0, -23), S(0, -20), S(3, -20), S(5, -26), S(4, -30), S(8, -31) }; // per file

constexpr ScoreExt openFileRookBonus[2] = { S(  10,   9), S(  34,   8) };

constexpr ScoreExt pawnSquareTable[64] = {
        S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
        S( -13,   7), S(  -4,   0), S(   1,   4), S(   6,   1), S(   3,  10), S(  -9,   4), S(  -9,   3), S( -16,   7),
        S( -21,   5), S( -17,   6), S(  -1,  -6), S(  12, -14), S(   8, -10), S(  -4,  -5), S( -15,   7), S( -24,  11),
        S( -14,  16), S( -21,  17), S(   9, -10), S(  10, -24), S(   4, -22), S(   4, -10), S( -20,  17), S( -17,  18),
        S( -15,  18), S( -18,  11), S( -16,  -8), S(   4, -30), S(  -2, -24), S( -18,  -9), S( -23,  13), S( -17,  21),
        S( -20,  48), S(  -9,  44), S(   1,  31), S(  17,  -9), S(  36,  -6), S(  -9,  31), S(  -6,  45), S( -23,  49),
        S( -33, -70), S( -66,  -9), S( -16, -22), S(  65, -23), S(  41, -18), S(  39, -14), S( -47,   4), S( -62, -51),
        S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0),
};

constexpr ScoreExt knightSquareTable[64] = {
        S( -31, -38), S(  -6, -24), S( -20, -22), S( -16,  -1), S( -11,  -1), S( -22, -19), S(  -8, -20), S( -41, -30),
        S(   1,  -5), S( -11,   3), S(  -6, -19), S(  -1,  -2), S(   0,   0), S(  -9, -16), S(  -8,  -3), S(  -6,   1),
        S(   7, -21), S(   8,  -5), S(   7,   2), S(  10,  19), S(  10,  19), S(   4,   2), S(   8,  -4), S(   3, -19),
        S(  16,  21), S(  17,  30), S(  23,  41), S(  27,  50), S(  24,  53), S(  23,  41), S(  19,  28), S(  13,  26),
        S(  13,  30), S(  23,  30), S(  37,  51), S(  30,  70), S(  26,  67), S(  38,  50), S(  22,  33), S(  14,  28),
        S( -24,  25), S(  -5,  37), S(  25,  56), S(  22,  60), S(  27,  55), S(  29,  55), S(  -1,  32), S( -19,  25),
        S(  13,  -2), S( -11,  18), S(  27,  -2), S(  37,  24), S(  41,  24), S(  40,  -7), S( -13,  16), S(   2,  -2),
        S(-167,  -5), S( -91,  12), S(-117,  41), S( -38,  17), S( -18,  19), S(-105,  48), S(-119,  24), S(-165, -17),
};

constexpr ScoreExt bishopSquareTable[64] = {
        S(   5, -21), S(   1,   1), S(  -1,   5), S(   1,   5), S(   2,   8), S(  -6,  -2), S(   0,   1), S(   4, -25),
        S(  26, -17), S(   2, -31), S(  15,  -2), S(   8,   8), S(   8,   8), S(  13,  -3), S(   9, -31), S(  26, -29),
        S(   9,   3), S(  22,   9), S(  -5,  -3), S(  18,  19), S(  17,  20), S(  -5,  -6), S(  20,   4), S(  15,   8),
        S(   0,  12), S(  10,  17), S(  17,  32), S(  20,  32), S(  24,  34), S(  12,  30), S(  15,  17), S(   0,  14),
        S( -20,  34), S(  13,  31), S(   1,  38), S(  21,  45), S(  12,  46), S(   6,  38), S(  13,  33), S( -14,  37),
        S( -13,  31), S( -11,  45), S(  -7,  23), S(   2,  40), S(   8,  38), S( -21,  34), S(  -5,  46), S(  -9,  35),
        S( -59,  38), S( -49,  22), S( -13,  30), S( -35,  36), S( -33,  36), S( -13,  33), S( -68,  21), S( -55,  35),
        S( -66,  18), S( -65,  36), S(-123,  48), S(-107,  56), S(-112,  53), S( -97,  43), S( -33,  22), S( -74,  15),
};

constexpr ScoreExt rookSquareTable[64] = {
        S( -26,  -1), S( -21,   3), S( -14,   4), S(  -6,  -4), S(  -5,  -4), S( -10,   3), S( -13,  -2), S( -22, -14),
        S( -70,   5), S( -25, -10), S( -18,  -7), S( -11, -11), S(  -9, -13), S( -15, -15), S( -15, -17), S( -77,   3),
        S( -39,   3), S( -16,  14), S( -25,   9), S( -14,   2), S( -12,   3), S( -25,   8), S(  -4,   9), S( -39,   1),
        S( -32,  24), S( -21,  36), S( -21,  36), S(  -5,  26), S(  -8,  27), S( -19,  34), S( -13,  33), S( -30,  24),
        S( -22,  46), S(   4,  38), S(  16,  38), S(  35,  30), S(  33,  32), S(  10,  36), S(  17,  31), S( -14,  43),
        S( -33,  60), S(  17,  41), S(   0,  54), S(  33,  36), S(  29,  35), S(   3,  52), S(  33,  32), S( -26,  56),
        S( -18,  41), S( -24,  47), S(  -1,  38), S(  15,  38), S(  14,  37), S(  -2,  36), S( -24,  49), S( -12,  38),
        S(  33,  55), S(  24,  63), S(  -1,  73), S(   9,  66), S(  10,  67), S(   0,  69), S(  34,  59), S(  37,  56),
};

constexpr ScoreExt queenSquareTable[64] = {
        S(  20, -34), S(   4, -26), S(   9, -34), S(  17, -16), S(  18, -18), S(  14, -46), S(   9, -28), S(  22, -44),
        S(   6, -15), S(  15, -22), S(  22, -42), S(  13,   2), S(  17,   0), S(  22, -49), S(  18, -29), S(   3, -18),
        S(   6,  -1), S(  21,   7), S(   5,  35), S(   0,  34), S(   2,  34), S(   5,  37), S(  24,   9), S(  13, -15),
        S(   9,  17), S(  12,  46), S(  -6,  59), S( -19, 109), S( -17, 106), S(  -4,  57), S(  18,  48), S(   8,  33),
        S( -10,  42), S(  -8,  79), S( -19,  66), S( -32, 121), S( -32, 127), S( -23,  80), S(  -8,  95), S( -10,  68),
        S( -28,  56), S( -23,  50), S( -33,  66), S( -18,  70), S( -17,  71), S( -19,  63), S( -18,  65), S( -28,  76),
        S( -16,  61), S( -72, 108), S( -19,  65), S( -52, 114), S( -54, 120), S( -14,  59), S( -69, 116), S( -11,  73),
        S(   8,  43), S(  19,  47), S(   0,  79), S(   3,  78), S(  -3,  89), S(  13,  65), S(  18,  79), S(  21,  56),
};

constexpr ScoreExt kingSquareTable[64] = {
        S(  87, -77), S(  67, -49), S(   4,  -7), S(  -9, -26), S( -10, -27), S(  -8,  -1), S(  57, -50), S(  79, -82),
        S(  35,   3), S( -27,  -3), S( -41,  16), S( -89,  29), S( -64,  26), S( -64,  28), S( -25,  -3), S(  30,  -4),
        S( -44, -19), S( -16, -19), S(  28,   7), S(   0,  35), S(  18,  32), S(  31,   9), S( -13, -18), S( -36, -13),
        S( -48, -44), S(  98, -39), S(  71,  12), S( -22,  45), S(  12,  41), S(  79,  10), S( 115, -34), S( -59, -38),
        S(  -6, -10), S(  95, -39), S(  39,  14), S( -49,  18), S( -27,  19), S(  35,  14), S(  81, -34), S( -50, -13),
        S(  24, -39), S( 123, -22), S( 105,  -1), S( -22, -21), S( -39, -20), S(  74, -15), S( 100, -23), S( -17, -49),
        S(   0, -98), S(  28, -21), S(   7, -18), S(  -3, -41), S( -57, -39), S(  12, -26), S(  22, -24), S( -15,-119),
        S( -16,-153), S(  49, -94), S( -21, -73), S( -19, -32), S( -51, -55), S( -42, -62), S(  53, -93), S( -58,-133),
};

constexpr const ScoreExt* pieceSquareTable[6] = {
        pawnSquareTable,
        knightSquareTable,
        bishopSquareTable,
        rookSquareTable,
        queenSquareTable,
        kingSquareTable
};

constexpr Score tempoBonus = 10;

constexpr std::int32_t knightPhaseValue = 1;
constexpr std::int32_t bishopPhaseValue = 1;
constexpr std::int32_t rookPhaseValue   = 2;
constexpr std::int32_t queenPhaseValue  = 4;

constexpr std::int32_t phaseMidGame = 24;

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

    std::int32_t phase;
};

void initEvaluationParameters();
Score evaluate(const Position& position);
void initializeEvaluationData(const Position& position, EvaluationData& evalData);
ScoreExt evaluateMaterial(EvaluationData& evaluationData);
template<Color color>
ScoreExt evaluatePawns(const Position& position);
template<PieceType pieceType, Color color>
ScoreExt evaluatePieces(const Position& position);
template<Color color>
ScoreExt evaluateKing(const Position& position);
void evaluatePhase(EvaluationData& evaluationData);
Score interpolateScore(ScoreExt finalScore, EvaluationData& evaluationData);
bool checkInsufficientMaterial(const Position& position);