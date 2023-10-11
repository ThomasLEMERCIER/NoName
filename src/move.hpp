#pragma once

#include "piece.hpp"
#include "square.hpp"

#include <cstdint>
#include <iostream>

/* Move encoding:

  binary                                                                        hexadecimal     shift

  0000 0000 0000 0000 0011 1111   source square (6 bits) (max val 63 (2^6-1))   0x3f            0
  0000 0000 0000 1111 1100 0000   target square (6 bits) (max val 63 (2^6-1))   0xfc0           6
  0000 0000 1111 0000 0000 0000   piece (4 bits) (max val 11 (2^4-1))           0xf000          12
  0000 1111 0000 0000 0000 0000   promoted piece (4 bits)                       0xf0000         16
  0001 0000 0000 0000 0000 0000   capture flag (1 bit)                          0x100000        20
  0010 0000 0000 0000 0000 0000   double push flag (1 bit)                      0x100000        21
  0100 0000 0000 0000 0000 0000   enpassant flag (1 bit)                        0x400000        22
  1000 0000 0000 0000 0000 0000   castling flag (1 bit)                         0x800000        23

  ==> need minimum 24 bits to store a move, so we use 32 bits (4 bytes)
*/


class Move
{
private:
    std::uint32_t value;

public:
    static Move Invalid() { return {0}; }
    static Move Null() { return {UINT32_MAX}; }

    Move() : value(0) {};
    Move(const std::uint32_t val) : value(val) {};

    Move(Square from, Square to, Piece piece, Piece promotionPiece, bool capture, bool doublePush, bool enpassant, bool castling) :
                                                                                    value((static_cast<std::uint32_t>(from.index())) |
                                                                                          (static_cast<std::uint32_t>(to.index()) << 6) |
                                                                                          (static_cast<std::uint32_t>(piece) << 12) |
                                                                                          (static_cast<std::uint32_t>(promotionPiece) << 16) |
                                                                                          (static_cast<std::uint32_t>(capture) << 20) |
                                                                                          (static_cast<std::uint32_t>(doublePush) << 21) |
                                                                                          (static_cast<std::uint32_t>(enpassant) << 22) |
                                                                                          (static_cast<std::uint32_t>(castling) << 23) ) {};

    Move(Square from, Square to, Piece piece, bool capture, bool doublePush, bool enpassant, bool castling) :
                                                                                    value((static_cast<std::uint32_t>(from.index())) |
                                                                                          (static_cast<std::uint32_t>(to.index()) << 6) |
                                                                                          (static_cast<std::uint32_t>(piece) << 12) |
                                                                                          (static_cast<std::uint32_t>(capture) << 20) |
                                                                                          (static_cast<std::uint32_t>(doublePush) << 21) |
                                                                                          (static_cast<std::uint32_t>(enpassant) << 22) |
                                                                                          (static_cast<std::uint32_t>(castling) << 23) ) {};

    constexpr Square getFrom() const { return static_cast<Square>(value & 0x3f); }
    constexpr Square getTo() const { return static_cast<Square>((value >> 6) & 0x3f); }
    constexpr Piece getPiece() const { return static_cast<Piece>((value >> 12) & 0xf); }
    constexpr Piece getPromotionPiece() const { return static_cast<Piece>((value >> 16) & 0xf); }
    constexpr bool isCapture() const { return (value >> 20) & 0x1; }
    constexpr bool isDoublePush() const { return (value >> 21) & 0x1; }
    constexpr bool isEnpassant() const { return (value >> 22) & 0x1; }
    constexpr bool isCastling() const { return (value >> 23) & 0x1; }

    constexpr bool isQuiet() const { return !isCapture() && getPromotionPiece() == static_cast<Piece>(0); }
    constexpr bool isValid() const { return value != 0u; }
    constexpr bool isNull() const { return value == UINT32_MAX; }

    constexpr bool operator==(const Move& rhs) const { return value == rhs.value; }

    friend std::ostream& operator<<(std::ostream& output, const Move& move) {
        if (!move.isValid()) {
            std::cout << "INVALID MOVE"; return output;
        }
        if (move.getPromotionPiece() != static_cast<Piece>(0)) {
            output << move.getFrom() << move.getTo() << pieceNames[static_cast<std::uint8_t>(move.getPromotionPiece())];
        }
        else {
            output << move.getFrom() << move.getTo();
        }
        return output;
    }
};
