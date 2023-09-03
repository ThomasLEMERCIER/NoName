#pragma once

#include "movelist.hpp"
#include "position.hpp"
#include "color.hpp"
#include "piece.hpp"
#include "square.hpp"
#include "attacks.hpp"

enum class MoveType {
    AllMoves,
    NonQuietMoves,
    QuietMoves,
};

template <MoveType move_type, Color color>
void generate_pawn_moves(MoveList& move_list, const Position& position) {
    constexpr Color opponent = ~color;
    constexpr Direction forward = (color == Color::White) ? Direction::North : Direction::South;
    constexpr Direction backward = (color == Color::White) ? Direction::South : Direction::North;
    constexpr Bitboard promotion_rank = (color == Color::White) ? Bitboard::RankBitboard(6) : Bitboard::RankBitboard(1);
    constexpr Bitboard double_push_rank = (color == Color::White) ? Bitboard::RankBitboard(2) : Bitboard::RankBitboard(5);
    constexpr Piece pawn = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;

    Bitboard pawns = position.get_pieces<pawn>();
    Bitboard pawns_on_promotion_rank = pawns & promotion_rank;
    Bitboard pawns_not_on_promotion_rank = pawns & ~promotion_rank;

    Bitboard empty_squares = ~position.occupied;
    Bitboard opponent_pieces = position.get_occupied<opponent>();

    Bitboard single_pushes = pawns_not_on_promotion_rank.shift<forward>() & ~position.occupied;
    Bitboard double_pushes = (single_pushes & double_push_rank).shift<forward>() & empty_squares;

    while (single_pushes) {
        Square to = single_pushes.PopLsb();
        Square from = to.shift<backward>();
        move_list.add_move(from, to, pawn);
    }

    while (double_pushes) {
        Square to = double_pushes.PopLsb();
        Square from = to.template shift<backward>().template shift<backward>();
        move_list.add_double_push(from, to, pawn);
    }

    if (pawns_on_promotion_rank) {
        Bitboard captures_right = pawns_on_promotion_rank.template shift<forward>().template shift<Direction::East>() & opponent_pieces;
        Bitboard captures_left = pawns_on_promotion_rank.template shift<forward>(). template shift<Direction::West>() & opponent_pieces;
        Bitboard non_captures = pawns_on_promotion_rank.shift<forward>() & empty_squares;

        while (captures_right) {
            Square to = captures_right.PopLsb();
            Square from = to.template shift<backward>().template shift<Direction::West>();
            move_list.add_promotion(from, to, pawn, true, color);
        }

        while (captures_left) {
            Square to = captures_left.PopLsb();
            Square from = to.template shift<backward>().template shift<Direction::East>();
            move_list.add_promotion(from, to, pawn, true, color);
        }

        while (non_captures) {
            Square to = non_captures.PopLsb();
            Square from = to.shift<backward>();
            move_list.add_promotion(from, to, pawn, color);
        }
    }

    Bitboard captures_right = pawns_not_on_promotion_rank.template shift<forward>().template shift<Direction::East>() & opponent_pieces;
    Bitboard captures_left = pawns_not_on_promotion_rank.template shift<forward>().template shift<Direction::West>() & opponent_pieces;

    while (captures_right) {
        Square to = captures_right.PopLsb();
        Square from = to.template shift<backward>().template shift<Direction::West>();
        move_list.add_move(from, to, pawn, true);
    }

    while (captures_left) {
        Square to = captures_left.PopLsb();
        Square from = to.template shift<backward>().template shift<Direction::East>();
        move_list.add_move(from, to, pawn, true);
    }

    if (position.en_passant_square != Square::None) {
        Bitboard pawn_able_to_capture = get_pawn_attacks(position.en_passant_square, opponent) & pawns_not_on_promotion_rank;

        while (pawn_able_to_capture) {
            Square from = pawn_able_to_capture.PopLsb();
            move_list.add_en_passant(from, position.en_passant_square, pawn);
        }
    }
}

template <MoveType move_type, Color color>
void generate_castling_moves(MoveList& move_list, const Position& position) {

    constexpr Color opponent = ~color;
    if constexpr (color == Color::White) {
        if ((position.castling_rights & CastlingRight::WhiteKingSide) != CastlingRight::None) {
            const Square sq1 = Square::F1;
            const Square sq2 = Square::G1;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.is_square_attacked_by<opponent>(Square::E1) && !position.is_square_attacked_by<opponent>(Square::F1) && !position.is_square_attacked_by<opponent>(Square::G1)) {
                    move_list.add_castling(Square::E1, Square::G1, Piece::WhiteKing);
                }
            }
        }

        if ((position.castling_rights & CastlingRight::WhiteQueenSide) != CastlingRight::None) {
            const Square sq1 = Square::B1;
            const Square sq2 = Square::C1;
            const Square sq3 = Square::D1;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2) | Bitboard(sq3);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.is_square_attacked_by<opponent>(Square::E1) && !position.is_square_attacked_by<opponent>(Square::D1) && !position.is_square_attacked_by<opponent>(Square::C1)) {
                    move_list.add_castling(Square::E1, Square::C1, Piece::WhiteKing);
                }
            }

        }
    }
    else {
        if ((position.castling_rights & CastlingRight::BlackKingSide) != CastlingRight::None) {
            const Square sq1 = Square::F8;
            const Square sq2 = Square::G8;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.is_square_attacked_by<opponent>(Square::E8) && !position.is_square_attacked_by<opponent>(Square::F8) && !position.is_square_attacked_by<opponent>(Square::G8)) {
                    move_list.add_castling(Square::E8, Square::G8, Piece::BlackKing);
                }
            }
        }

        if ((position.castling_rights & CastlingRight::BlackQueenSide) != CastlingRight::None) {
            const Square sq1 = Square::B8;
            const Square sq2 = Square::C8;
            const Square sq3 = Square::D8;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2) | Bitboard(sq3);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.is_square_attacked_by<opponent>(Square::E8) && !position.is_square_attacked_by<opponent>(Square::D8) && !position.is_square_attacked_by<opponent>(Square::C8)) {
                    move_list.add_castling(Square::E8, Square::C8, Piece::BlackKing);
                }
            }

        }
    }
}

template <MoveType move_type, Piece piece>
void generate_piece_moves(MoveList& move_list, const Position& position) {
    constexpr Color color = get_piece_color(piece);
    constexpr PieceType piece_type = get_piece_type(piece);

    Bitboard pieces = position.get_pieces<piece>();
    while (pieces) {
        Square from = pieces.PopLsb();
        Bitboard attacks = get_attacks<piece_type, color>(from, position.occupied);
        Bitboard targets = attacks & ~position.get_occupied<color>();
        while (targets) {
            Square to = targets.PopLsb();
            move_list.add_move(from, to, piece, static_cast<bool>(position.occupied & to));
        }
    }
}

template <MoveType move_type>
void generate_moves(MoveList& move_list, const Position& position) {

    if (position.side_to_move == Color::White) {
        generate_pawn_moves<move_type, Color::White>(move_list, position);
        generate_castling_moves<move_type, Color::White>(move_list, position);
        generate_piece_moves<move_type, Piece::WhiteKnight>(move_list, position);
        generate_piece_moves<move_type, Piece::WhiteBishop>(move_list, position);
        generate_piece_moves<move_type, Piece::WhiteRook>(move_list, position);
        generate_piece_moves<move_type, Piece::WhiteQueen>(move_list, position);
        generate_piece_moves<move_type, Piece::WhiteKing>(move_list, position);
    }
    else {
        generate_pawn_moves<move_type, Color::Black>(move_list, position);
        generate_castling_moves<move_type, Color::Black>(move_list, position);
        generate_piece_moves<move_type, Piece::BlackKnight>(move_list, position);
        generate_piece_moves<move_type, Piece::BlackBishop>(move_list, position);
        generate_piece_moves<move_type, Piece::BlackRook>(move_list, position);
        generate_piece_moves<move_type, Piece::BlackQueen>(move_list, position);
        generate_piece_moves<move_type, Piece::BlackKing>(move_list, position);
    }
}
