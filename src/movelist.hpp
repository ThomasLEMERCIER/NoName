#pragma once

#include <cstdint>
#include <iostream>

#include "move.hpp"
#include "piece.hpp"

class MoveList
{
private:
    Move moves[256];
    std::uint32_t size;

public:
    MoveList() : size(0) {};

    void add_move(const Move move) { moves[size++] = move; }
    void add_move(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, false, false, false, false); }
    void add_move(const Square from, const Square to, const Piece piece, const bool capture) { moves[size++] = Move(from, to, piece, capture, false, false, false); }

    void add_en_passant(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, true, false, true, false); }
    void add_double_push(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, false, true, false, false); }
    void add_castling(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, false, false, false, true); }


    void add_promotion(const Square from, const Square to, const Piece piece, const Color color) { if (color == Color::White) { 
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteQueen, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteRook, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteBishop, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteKnight, false, false, false, false); }
                                                                                                    else { 
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackQueen, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackRook, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackBishop, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackKnight, false, false, false, false); } }

    void add_promotion(const Square from, const Square to, const Piece piece, const bool capture, const Color color) { if (color == Color::White) { 
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteQueen, capture, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteRook, capture, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteBishop, capture, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteKnight, capture, false, false, false); }
                                                                                                    else { 
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackQueen, capture, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackRook, capture, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackBishop, capture, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackKnight, capture, false, false, false); } }


    void clear() { size = 0; }

    std::uint32_t get_size() const { return size; }
    Move get_move(const std::uint32_t index) const { return moves[index]; }
    Move operator[](const std::uint32_t index) const { return moves[index]; }

    friend std::ostream& operator<<(std::ostream& output, const MoveList& move_list) {
        for (std::uint32_t i = 0; i < move_list.size; ++i) {
            output << move_list.moves[i] << "\n";
        }
        return output;
    }
};
