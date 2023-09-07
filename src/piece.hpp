#pragma once

#include "color.hpp"

#include <cstdint>
#include <string_view>
#include <unordered_map>

enum class PieceType : std::uint8_t {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
};

enum class Piece : std::uint8_t {
    WhitePawn = 0,
    WhiteKnight = 1,
    WhiteBishop = 2,
    WhiteRook = 3,
    WhiteQueen = 4,
    WhiteKing = 5,
    BlackPawn = 6,
    BlackKnight = 7,
    BlackBishop = 8,
    BlackRook = 9,
    BlackQueen = 10,
    BlackKing = 11,
    None = 12
};

constexpr std::string_view pieceNames[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", "." };
const std::unordered_map<char, Piece> pieceChars = {
    { 'P', Piece::WhitePawn },
    { 'N', Piece::WhiteKnight },
    { 'B', Piece::WhiteBishop },
    { 'R', Piece::WhiteRook },
    { 'Q', Piece::WhiteQueen },
    { 'K', Piece::WhiteKing },
    { 'p', Piece::BlackPawn },
    { 'n', Piece::BlackKnight },
    { 'b', Piece::BlackBishop },
    { 'r', Piece::BlackRook },
    { 'q', Piece::BlackQueen },
    { 'k', Piece::BlackKing },
    { '.', Piece::None }
};

constexpr PieceType getPieceType(const Piece piece) {
    return static_cast<PieceType>(static_cast<std::uint8_t>(piece) % 6);
}

constexpr Color getPieceColor(const Piece piece) {
    return static_cast<Color>(static_cast<std::uint8_t>(piece) / 6);
}

constexpr Piece getPiece(const PieceType pieceType, Color color) {
    return static_cast<Piece>(static_cast<std::uint8_t>(pieceType) + 6 * static_cast<std::uint8_t>(color));
}
