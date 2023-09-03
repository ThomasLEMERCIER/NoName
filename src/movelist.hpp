#pragma once

#include "move.hpp"
#include "piece.hpp"

#include <cstdint>
#include <iostream>

class MoveList
{
private:
    Move moves[256];
    std::uint32_t size;

public:
    MoveList() : size(0) {};

    void addMove(const Move move) { moves[size++] = move; }
    void addMove(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, false, false, false, false); }
    void addMove(const Square from, const Square to, const Piece piece, const bool capture) { moves[size++] = Move(from, to, piece, capture, false, false, false); }

    void addEnPassant(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, true, false, true, false); }
    void addDoublePush(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, false, true, false, false); }
    void addCastling(const Square from, const Square to, const Piece piece) { moves[size++] = Move(from, to, piece, false, false, false, true); }


    void addPromotion(const Square from, const Square to, const Piece piece, const Color color) { if (color == Color::White) {
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteQueen, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteRook, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteBishop, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::WhiteKnight, false, false, false, false); }
                                                                                                  else {
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackQueen, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackRook, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackBishop, false, false, false, false);
                                                                                                        moves[size++] = Move(from, to, piece, Piece::BlackKnight, false, false, false, false); } }

    void addPromotion(const Square from, const Square to, const Piece piece, const bool capture, const Color color) { if (color == Color::White) {
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

    std::uint32_t getSize() const { return size; }
    Move operator[](const std::uint32_t index) const { return moves[index]; }

    friend std::ostream& operator<<(std::ostream& output, const MoveList& moveList) {
        for (std::uint32_t i = 0; i < moveList.size; ++i) {
            output << moveList.moves[i] << "\n";
        }
        return output;
    }
};
