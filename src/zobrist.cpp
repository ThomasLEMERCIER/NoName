#include "zobrist.hpp"

#include "rng.hpp"

std::uint64_t pieceSquareZobristHash[12][64];
std::uint64_t colorZobristHash;
std::uint64_t enPassantFileZobristHash[8];
std::uint64_t castlingRightZobristHash[16];

void initZobristKeys() {
    PRNG rng { 0x0123456789abcdef };

    for (auto & piece : pieceSquareZobristHash) {
        for (std::uint64_t& square : piece) {
            square = rng.next();
        }
    }

    colorZobristHash = rng.next();

    for (std::uint64_t& file : enPassantFileZobristHash) {
        file = rng.next();
    }

    for (std::uint64_t& castlingRight : castlingRightZobristHash) {
        castlingRight = rng.next();
    }
}

std::uint64_t getPieceSquareHash(const Color color, const PieceType pieceType, const Square square)  {
    return pieceSquareZobristHash[static_cast<uint8_t>(getPiece(pieceType, color))][square.index()];
}
