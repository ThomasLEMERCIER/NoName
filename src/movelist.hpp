#pragma once

#include "move.hpp"
#include "piece.hpp"

#include <cstdint>
#include <iostream>

struct MoveExt {
    Move move;
    std::int32_t score = INT32_MIN;
};

class MoveList
{
private:
    MoveExt moves[256];
    std::uint32_t size;

public:
    MoveList() : size(0) {};

    void addMove(const Move move) { moves[size++] = {move, INT32_MIN}; }
    void addMove(const Square from, const Square to, const Piece piece) { addMove(Move(from, to, piece, false, false, false, false)); }
    void addMove(const Square from, const Square to, const Piece piece, const bool capture) { addMove(Move(from, to, piece, capture, false, false, false)); }

    void addEnPassant(const Square from, const Square to, const Piece piece) { addMove(Move(from, to, piece, true, false, true, false)); }
    void addDoublePush(const Square from, const Square to, const Piece piece) { addMove(Move(from, to, piece, false, true, false, false)); }
    void addCastling(const Square from, const Square to, const Piece piece) { addMove(Move(from, to, piece, false, false, false, true)); }


    void addPromotion(const Square from, const Square to, const Piece piece, const Color color) { if (color == Color::White) {
            addMove(Move(from, to, piece, Piece::WhiteQueen, false, false, false, false));
            addMove(Move(from, to, piece, Piece::WhiteRook, false, false, false, false));
            addMove(Move(from, to, piece, Piece::WhiteBishop, false, false, false, false));
            addMove(Move(from, to, piece, Piece::WhiteKnight, false, false, false, false)); }
                                                                                                  else {
            addMove(Move(from, to, piece, Piece::BlackQueen, false, false, false, false));
            addMove(Move(from, to, piece, Piece::BlackRook, false, false, false, false));
            addMove(Move(from, to, piece, Piece::BlackBishop, false, false, false, false));
            addMove(Move(from, to, piece, Piece::BlackKnight, false, false, false, false)); } }

    void addPromotion(const Square from, const Square to, const Piece piece, const bool capture, const Color color) { if (color == Color::White) {
            addMove(Move(from, to, piece, Piece::WhiteQueen, capture, false, false, false));
            addMove(Move(from, to, piece, Piece::WhiteRook, capture, false, false, false));
            addMove(Move(from, to, piece, Piece::WhiteBishop, capture, false, false, false));
            addMove(Move(from, to, piece, Piece::WhiteKnight, capture, false, false, false)); }
                                                                                                                      else {
            addMove(Move(from, to, piece, Piece::BlackQueen, capture, false, false, false));
            addMove(Move(from, to, piece, Piece::BlackRook, capture, false, false, false));
            addMove(Move(from, to, piece, Piece::BlackBishop, capture, false, false, false));
            addMove(Move(from, to, piece, Piece::BlackKnight, capture, false, false, false)); } }


    void clear() { size = 0; }
    std::uint32_t getSize() const { return size; }
    MoveExt& operator[](const std::uint32_t index) { return moves[index]; }

    friend std::ostream& operator<<(std::ostream& output, const MoveList& moveList) {
        for (std::uint32_t i = 0; i < moveList.size; ++i) {
            output << moveList.moves[i].move << ", " << moveList.moves[i].score << "\n";
        }
        return output;
    }
};
