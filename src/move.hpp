#pragma once

#include <cstdint>
#include <iostream>

#include "square.hpp"
#include "piece.hpp"

/* Move encoding:

  binary                                                                        hexidecimal     shift

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
    Move() : value(0) {};
    Move(const std::uint32_t val) : value(val) {};

    Move(Square from, Square to, Piece piece, Piece promotion_piece, bool capture, bool double_push, bool enpassant, bool castling) : 
                                                                                    value(  (static_cast<std::uint32_t>(from.index())) |
                                                                                            (static_cast<std::uint32_t>(to.index()) << 6) |
                                                                                            (static_cast<std::uint32_t>(piece) << 12) |
                                                                                            (static_cast<std::uint32_t>(promotion_piece) << 16) |
                                                                                            (static_cast<std::uint32_t>(capture) << 20) |
                                                                                            (static_cast<std::uint32_t>(double_push) << 21) |
                                                                                            (static_cast<std::uint32_t>(enpassant) << 22) |
                                                                                            (static_cast<std::uint32_t>(castling) << 23) ) {};

    Move(Square from, Square to, Piece piece, bool capture, bool double_push, bool enpassant, bool castling) :
                                                                                    value(  (static_cast<std::uint32_t>(from.index())) |
                                                                                            (static_cast<std::uint32_t>(to.index()) << 6) |
                                                                                            (static_cast<std::uint32_t>(piece) << 12) |
                                                                                            (static_cast<std::uint32_t>(capture) << 20) |
                                                                                            (static_cast<std::uint32_t>(double_push) << 21) |
                                                                                            (static_cast<std::uint32_t>(enpassant) << 22) |
                                                                                            (static_cast<std::uint32_t>(castling) << 23) ) {};

    constexpr Square get_from() const { return static_cast<Square>(value & 0x3f); }
    constexpr Square get_to() const { return static_cast<Square>((value >> 6) & 0x3f); }
    constexpr Piece get_piece() const { return static_cast<Piece>((value >> 12) & 0xf); }
    constexpr Piece get_promotion_piece() const { return static_cast<Piece>((value >> 16) & 0xf); }
    constexpr bool is_capture() const { return (value >> 20) & 0x1; }
    constexpr bool is_double_push() const { return (value >> 21) & 0x1; }
    constexpr bool is_enpassant() const { return (value >> 22) & 0x1; }
    constexpr bool is_castling() const { return (value >> 23) & 0x1; }

    friend std::ostream& operator<<(std::ostream& output, const Move& move) {
        if (move.get_promotion_piece() != static_cast<Piece>(0)) {
            output << move.get_from() << move.get_to() << PieceNames[static_cast<std::uint8_t>(move.get_promotion_piece())];
        }
        else {
            output << move.get_from() << move.get_to();
        }
        return output;
    }
};
