#pragma once

#include "bitboard.hpp"
#include "color.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "square.hpp"

#include <array>
#include <string>

enum class CastlingRight : std::uint8_t {
    WhiteKingSide = 1,
    WhiteQueenSide = 2,
    BlackKingSide = 4,
    BlackQueenSide = 8,
    None = 0,

    WhiteKingMoved = 12,
    WhiteKingRookMoved = 14,
    WhiteQueenRookMoved = 13,

    BlackKingMoved = 3,
    BlackKingRookMoved = 11,
    BlackQueenRookMoved = 7,

    Rest = 15
};

constexpr CastlingRight operator|(const CastlingRight lhs, const CastlingRight rhs) { return static_cast<CastlingRight>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)); }
constexpr CastlingRight operator&(const CastlingRight lhs, const CastlingRight rhs) { return static_cast<CastlingRight>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)); }
constexpr CastlingRight operator~(const CastlingRight rhs) { return static_cast<CastlingRight>(~static_cast<uint8_t>(rhs)); }
constexpr CastlingRight& operator|=(CastlingRight& lhs, const CastlingRight rhs) { return lhs = static_cast<CastlingRight>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)); }
constexpr CastlingRight& operator&=(CastlingRight& lhs, const CastlingRight rhs) { return lhs = static_cast<CastlingRight>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)); }
constexpr bool operator==(const CastlingRight lhs, const CastlingRight rhs) { return static_cast<uint8_t>(lhs) == static_cast<uint8_t>(rhs); }

constexpr CastlingRight castlingRightUpdate[64] = {
    CastlingRight::WhiteQueenRookMoved, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::WhiteKingMoved, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::WhiteKingRookMoved,
    CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,
    CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,
    CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,
    CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,
    CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,
    CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,
    CastlingRight::BlackQueenRookMoved, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::Rest,  CastlingRight::BlackKingMoved, CastlingRight::Rest, CastlingRight::Rest, CastlingRight::BlackKingRookMoved,
};

struct SidePosition // substructure of Position containing all pieces of one color
{
    Bitboard& operator[](const PieceType piece) {
        return pieces[static_cast<std::uint8_t>(piece)];
    }
    Bitboard operator[](const PieceType piece) const {
        return pieces[static_cast<std::uint8_t>(piece)];
    }
    
    std::array<Bitboard, 6> pieces {0ULL}; // array of bitboards for each piece type
    Bitboard occupied {0};  // all squares occupied by side pieces

    Bitboard& pawns = pieces[0];
    Bitboard& knights = pieces[1];
    Bitboard& bishops = pieces[2];
    Bitboard& rooks = pieces[3];
    Bitboard& queens = pieces[4];
    Bitboard& king = pieces[5];

    SidePosition(const SidePosition& other)
        : pieces(other.pieces), occupied(other.occupied), pawns(pieces[0]),
          knights(pieces[1]), bishops(pieces[2]), rooks(pieces[3]),
          queens(pieces[4]), king(pieces[5]) {}
    SidePosition() = default;
};


class Position
{
public:
    SidePosition white;               // pieces information for white
    SidePosition black;               // pieces information for black

    Bitboard occupied;                // all squares occupied by any piece

    Color sideToMove;               // side to move
    Square enPassantSquare;         // en passant square
    CastlingRight castlingRights;    // castling rights (encoded as bits) 0001 = white king side, 0010 = white queen side, 0100 = black king side, 1000 = black queen side

    Position() : sideToMove(Color::White), enPassantSquare(Square::None), castlingRights(CastlingRight::None) {};
    Position(const Position& pos) = default;
        
    void loadFromFen(const std::string& fen);                                     // load position from FEN string

    void setPiece(const Color color, const PieceType piece, const Square square);  // set piece at given square
    void removePiece(const Color color, const PieceType piece, const Square square);   // remove piece at given square
    Piece pieceAt(const Square square) const;                                      // return piece at given square

    bool makeMove(const Move move);                                                // make move on position
    
    template<Piece piece>
    constexpr Bitboard getPieces() const {
        constexpr Color color = getPieceColor(piece);
        constexpr PieceType pieceType = getPieceType(piece);
        
        if constexpr (color == Color::White) { return white[pieceType]; }
        if constexpr (color == Color::Black) { return black[pieceType]; }
    }
    template<Color color>
    constexpr Bitboard getOccupied() const {
        if constexpr (color == Color::White) { return white.occupied; }
        if constexpr (color == Color::Black) { return black.occupied; }
    }

    template<Color color>
    bool isSquareAttackedBy(const Square square) const;                           // check if square is attacked by given color
    bool isInCheck(const Color color) const;                                       // check if given color is in check

    friend std::ostream& operator<<(std::ostream& output, const Position& pos);     // output operator
};
