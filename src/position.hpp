#pragma once

#include <string>
#include <array>

#include "bitboard.hpp"
#include "color.hpp"
#include "piece.hpp"
#include "square.hpp"
#include "move.hpp"

enum class CastlingRight : std::uint8_t {
    WhiteKingSide = 1,
    WhiteQueenSide = 2,
    BlackKingSide = 4,
    BlackQueenSide = 8,
    None = 0,

    White = WhiteKingSide | WhiteQueenSide,
    Black = BlackKingSide | BlackQueenSide,

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

constexpr CastlingRight CastlingRightUpdate[64] = {
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


    // Bitboard& operator[](const PieceType piece) {
    //     if (piece == PieceType::Pawn) { return pawns; }
    //     if (piece == PieceType::Knight) { return knights; }
    //     if (piece == PieceType::Bishop) { return bishops; }
    //     if (piece == PieceType::Rook) { return rooks; }
    //     if (piece == PieceType::Queen) { return queens; }
    //     if (piece == PieceType::King) { return king; }
    // }

    // Bitboard operator[](const PieceType piece) const {
    //     if (piece == PieceType::Pawn) { return pawns; }
    //     if (piece == PieceType::Knight) { return knights; }
    //     if (piece == PieceType::Bishop) { return bishops; }
    //     if (piece == PieceType::Rook) { return rooks; }
    //     if (piece == PieceType::Queen) { return queens; }
    //     if (piece == PieceType::King) { return king; }
    // }

    // Bitboard pawns {0};
    // Bitboard knights {0};
    // Bitboard bishops {0};
    // Bitboard rooks {0};
    // Bitboard queens {0};
    // Bitboard king {0};

    // Bitboard occupied {0};  // all squares occupied by side pieces

    // Bitboard& operator[](const PieceType piece) {
    //     return *pieces[static_cast<std::uint8_t>(piece)];
    // }

    // Bitboard operator[](const PieceType piece) const {
    //     return *pieces[static_cast<std::uint8_t>(piece)];
    // }

    // Bitboard pawns {0};
    // Bitboard knights {0};
    // Bitboard bishops {0};
    // Bitboard rooks {0};
    // Bitboard queens {0};
    // Bitboard king {0};

    // Bitboard occupied {0};  // all squares occupied by side pieces

    // Bitboard* pieces[6] {&pawns, &knights, &bishops, &rooks, &queens, &king};
};


class Position
{
public:
    SidePosition white;               // pieces information for white
    SidePosition black;               // pieces information for black

    Bitboard occupied;                // all squares occupied by any piece

    Color side_to_move;               // side to move
    Square en_passant_square;         // en passant square
    CastlingRight castling_rights;    // castling rights (encoded as bits) 0001 = white king side, 0010 = white queen side, 0100 = black king side, 1000 = black queen side

    Position() : side_to_move(Color::White), en_passant_square(Square::None), castling_rights(CastlingRight::None) {};
    Position(const Position& pos) = default;
        
    void load_from_fen(const std::string& fen);                                     // load position from FEN string

    void set_piece(const Color color, const PieceType piece, const Square square);  // set piece at given square
    void remove_piece(const Color color, const PieceType piece, const Square square);   // remove piece at given square
    Piece piece_at(const Square square) const;                                      // return piece at given square

    bool make_move(const Move move);                                                // make move on position
    
    template<Piece piece>
    constexpr Bitboard get_pieces() const {
        constexpr Color color = get_piece_color(piece);
        constexpr PieceType piece_type = get_piece_type(piece);
        
        if constexpr (color == Color::White) { return white[piece_type]; }
        if constexpr (color == Color::Black) { return black[piece_type]; }
    }
    template<Color color>
    constexpr Bitboard get_occupied() const {
        if constexpr (color == Color::White) { return white.occupied; }
        if constexpr (color == Color::Black) { return black.occupied; }
    }

    template<Color color>
    bool is_square_attacked_by(const Square square) const;                           // check if square is attacked by given color
    bool is_in_check(const Color color) const;                                       // check if given color is in check

    friend std::ostream& operator<<(std::ostream& output, const Position& pos);     // output operator
};
