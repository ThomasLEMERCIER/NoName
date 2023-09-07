#pragma once

#include "color.hpp"
#include "piece.hpp"
#include "square.hpp"

#include <cstdint>

extern std::uint64_t pieceSquareZobristHash[12][64];
extern std::uint64_t colorZobristHash;
extern std::uint64_t enPassantFileZobristHash[8];
extern std::uint64_t castlingRightZobristHash[16];

void initZobristKeys();
std::uint64_t getPieceSquareHash(const Color color, const PieceType pieceType, const Square square);
