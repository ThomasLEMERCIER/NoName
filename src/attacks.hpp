#pragma once

#include "bitboard.hpp"
#include "square.hpp"
#include "color.hpp"
#include "piece.hpp"

enum class SlidingPiece {
    Rook,
    Bishop
};

extern Bitboard KnightAttacks[64];
extern Bitboard KingAttacks[64];
extern Bitboard PawnAttacks[2][64];
extern Bitboard RookAttacks[64][4096];
extern Bitboard BishopAttacks[64][512];

extern Bitboard RookMasks[64];
extern Bitboard BishopMasks[64];

extern Bitboard RookMagics[64];
extern Bitboard BishopMagics[64];
extern std::uint8_t RookMagicShifts[64];
extern std::uint8_t BishopMagicShifts[64];

void init_attacks();
void init_knight_attacks();
void init_king_attacks();
void init_pawn_attacks();
void init_rook_attacks();
void init_bishop_attacks();
void init_sliding_attacks(SlidingPiece piece);

void init_rook_masks();
void init_bishop_masks();

void init_rook_shifts();
void init_bishop_shifts();

void init_rook_magics();
void init_bishop_magics();

Bitboard find_magic(Square square, SlidingPiece piece);

Bitboard get_knight_attacks(Square square);
Bitboard get_king_attacks(Square square);
Bitboard get_pawn_attacks(Square square, Color color);
Bitboard get_rook_attacks(Square square, Bitboard occupied);
Bitboard get_bishop_attacks(Square square, Bitboard occupied);
Bitboard get_queen_attacks(Square square, Bitboard occupied);

Bitboard get_rook_attacks_OTF(Square square, Bitboard occupied);
Bitboard get_bishop_attacks_OTF(Square square, Bitboard occupied);

template<PieceType piece, Color color>
constexpr Bitboard get_attacks(Square square, Bitboard occupied) {
    if constexpr (piece == PieceType::Knight)   { return get_knight_attacks(square); }
    if constexpr (piece == PieceType::King)     { return get_king_attacks(square); }
    if constexpr (piece == PieceType::Rook)     { return get_rook_attacks(square, occupied); }
    if constexpr (piece == PieceType::Bishop)   { return get_bishop_attacks(square, occupied); }
    if constexpr (piece == PieceType::Queen)    { return get_queen_attacks(square, occupied); }
    if constexpr (piece == PieceType::Pawn)     { return get_pawn_attacks(square, color); }
}
