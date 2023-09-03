#pragma once

#include "bitboard.hpp"
#include "color.hpp"
#include "piece.hpp"
#include "square.hpp"

#include <cstdint>

enum class SlidingPiece : std::uint8_t {
    Rook,
    Bishop
};

extern Bitboard knightAttacks[64];
extern Bitboard kingAttacks[64];
extern Bitboard pawnAttacks[2][64];
extern Bitboard rookAttacks[64][4096];
extern Bitboard bishopAttacks[64][512];

extern Bitboard rookMasks[64];
extern Bitboard bishopMasks[64];

extern Bitboard rookMagics[64];
extern Bitboard bishopMagics[64];
extern std::uint8_t rookMagicShifts[64];
extern std::uint8_t bishopMagicShifts[64];

void initAttacks();
void initKnightAttacks();
void initKingAttacks();
void initPawnAttacks();
void initRookAttacks();
void initBishopAttacks();
void initSlidingAttacks(SlidingPiece piece);

void initRookMasks();
void initBishopMasks();

void initRookShifts();
void initBishopShifts();

void initRookMagics();
void initBishopMagics();

Bitboard findMagic(Square square, SlidingPiece piece);

Bitboard getKnightAttacks(Square square);
Bitboard getKingAttacks(Square square);
Bitboard getPawnAttacks(Square square, Color color);
Bitboard getRookAttacks(Square square, Bitboard occupied);
Bitboard getBishopAttacks(Square square, Bitboard occupied);
Bitboard getQueenAttacks(Square square, Bitboard occupied);

Bitboard getRookAttacksOTF(Square square, Bitboard occupied);
Bitboard getBishopAttacksOTF(Square square, Bitboard occupied);

template<PieceType piece, Color color>
constexpr Bitboard getAttacks(Square square, Bitboard occupied) {
    if constexpr (piece == PieceType::Knight)   { return getKnightAttacks(square); }
    if constexpr (piece == PieceType::King)     { return getKingAttacks(square); }
    if constexpr (piece == PieceType::Rook)     { return getRookAttacks(square, occupied); }
    if constexpr (piece == PieceType::Bishop)   { return getBishopAttacks(square, occupied); }
    if constexpr (piece == PieceType::Queen)    { return getQueenAttacks(square, occupied); }
    if constexpr (piece == PieceType::Pawn)     { return getPawnAttacks(square, color); }
}
