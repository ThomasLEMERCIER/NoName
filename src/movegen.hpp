#pragma once

#include "attacks.hpp"
#include "color.hpp"
#include "movelist.hpp"
#include "piece.hpp"
#include "position.hpp"
#include "square.hpp"

enum class MoveType {
    AllMoves,
    NonQuietMoves,
    QuietMoves,
};

template <MoveType moveType, Color color>
void generatePawnMoves(MoveList& moveList, const Position& position) {
    constexpr Color opponent = ~color;
    constexpr Direction forward = (color == Color::White) ? Direction::North : Direction::South;
    constexpr Direction backward = (color == Color::White) ? Direction::South : Direction::North;
    constexpr Bitboard promotionRank = (color == Color::White) ? Bitboard::RankBitboard(6) : Bitboard::RankBitboard(1);
    constexpr Bitboard doublePushRank = (color == Color::White) ? Bitboard::RankBitboard(2) : Bitboard::RankBitboard(5);
    constexpr Piece pawn = (color == Color::White) ? Piece::WhitePawn : Piece::BlackPawn;

    Bitboard pawns = position.getPieces<pawn>();
    Bitboard pawnsOnPromotionRank = pawns & promotionRank;
    Bitboard pawnsNotOnPromotionRank = pawns & ~promotionRank;

    Bitboard emptySquares = ~position.occupied;
    Bitboard opponentPieces = position.getOccupied<opponent>();

    Bitboard singlePushes = pawnsNotOnPromotionRank.shift<forward>() & ~position.occupied;
    Bitboard doublePushes = (singlePushes & doublePushRank).shift<forward>() & emptySquares;

    while (singlePushes) {
        Square to = singlePushes.popLsb();
        Square from = to.shift<backward>();
        moveList.addMove(from, to, pawn);
    }

    while (doublePushes) {
        Square to = doublePushes.popLsb();
        Square from = to.template shift<backward>().template shift<backward>();
        moveList.addDoublePush(from, to, pawn);
    }

    if (pawnsOnPromotionRank) {
        Bitboard capturesRight = pawnsOnPromotionRank.template shift<forward>().template shift<Direction::East>() & opponentPieces;
        Bitboard capturesLeft = pawnsOnPromotionRank.template shift<forward>(). template shift<Direction::West>() & opponentPieces;
        Bitboard nonCaptures = pawnsOnPromotionRank.shift<forward>() & emptySquares;

        while (capturesRight) {
            Square to = capturesRight.popLsb();
            Square from = to.template shift<backward>().template shift<Direction::West>();
            moveList.addPromotion(from, to, pawn, true, color);
        }

        while (capturesLeft) {
            Square to = capturesLeft.popLsb();
            Square from = to.template shift<backward>().template shift<Direction::East>();
            moveList.addPromotion(from, to, pawn, true, color);
        }

        while (nonCaptures) {
            Square to = nonCaptures.popLsb();
            Square from = to.shift<backward>();
            moveList.addPromotion(from, to, pawn, color);
        }
    }

    Bitboard capturesRight = pawnsNotOnPromotionRank.template shift<forward>().template shift<Direction::East>() & opponentPieces;
    Bitboard capturesLeft = pawnsNotOnPromotionRank.template shift<forward>().template shift<Direction::West>() & opponentPieces;

    while (capturesRight) {
        Square to = capturesRight.popLsb();
        Square from = to.template shift<backward>().template shift<Direction::West>();
        moveList.addMove(from, to, pawn, true);
    }

    while (capturesLeft) {
        Square to = capturesLeft.popLsb();
        Square from = to.template shift<backward>().template shift<Direction::East>();
        moveList.addMove(from, to, pawn, true);
    }

    if (position.enPassantSquare != Square::None) {
        Bitboard pawnAbleToCapture =
                getPawnAttacks(position.enPassantSquare, opponent) & pawnsNotOnPromotionRank;

        while (pawnAbleToCapture) {
            Square from = pawnAbleToCapture.popLsb();
            moveList.addEnPassant(from, position.enPassantSquare, pawn);
        }
    }
}

template <MoveType moveType, Color color>
void generateCastlingMoves(MoveList& moveList, const Position& position) {

    constexpr Color opponent = ~color;
    if constexpr (color == Color::White) {
        if ((position.castlingRights & CastlingRight::WhiteKingSide) != CastlingRight::None) {
            const Square sq1 = Square::F1;
            const Square sq2 = Square::G1;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.isSquareAttackedBy<opponent>(Square::E1) && !position.isSquareAttackedBy<opponent>(
                        Square::F1) && !position.isSquareAttackedBy<opponent>(
                        Square::G1)) {
                    moveList.addCastling(Square::E1, Square::G1, Piece::WhiteKing);
                }
            }
        }

        if ((position.castlingRights & CastlingRight::WhiteQueenSide) != CastlingRight::None) {
            const Square sq1 = Square::B1;
            const Square sq2 = Square::C1;
            const Square sq3 = Square::D1;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2) | Bitboard(sq3);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.isSquareAttackedBy<opponent>(Square::E1) && !position.isSquareAttackedBy<opponent>(
                        Square::D1) && !position.isSquareAttackedBy<opponent>(
                        Square::C1)) {
                    moveList.addCastling(Square::E1, Square::C1, Piece::WhiteKing);
                }
            }

        }
    }
    else {
        if ((position.castlingRights & CastlingRight::BlackKingSide) != CastlingRight::None) {
            const Square sq1 = Square::F8;
            const Square sq2 = Square::G8;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.isSquareAttackedBy<opponent>(Square::E8) && !position.isSquareAttackedBy<opponent>(
                        Square::F8) && !position.isSquareAttackedBy<opponent>(
                        Square::G8)) {
                    moveList.addCastling(Square::E8, Square::G8, Piece::BlackKing);
                }
            }
        }

        if ((position.castlingRights & CastlingRight::BlackQueenSide) != CastlingRight::None) {
            const Square sq1 = Square::B8;
            const Square sq2 = Square::C8;
            const Square sq3 = Square::D8;
            const Bitboard between = Bitboard(sq1) | Bitboard(sq2) | Bitboard(sq3);
            const Bitboard occupiedSquares = position.occupied & between;
            if (occupiedSquares == 0ULL) {
                if (!position.isSquareAttackedBy<opponent>(Square::E8) && !position.isSquareAttackedBy<opponent>(
                        Square::D8) && !position.isSquareAttackedBy<opponent>(
                        Square::C8)) {
                    moveList.addCastling(Square::E8, Square::C8, Piece::BlackKing);
                }
            }

        }
    }
}

template <MoveType moveType, Piece piece>
void generatePieceMoves(MoveList& moveList, const Position& position) {
    constexpr Color color = getPieceColor(piece);
    constexpr PieceType pieceType = getPieceType(piece);

    Bitboard pieces = position.getPieces<piece>();
    while (pieces) {
        Square from = pieces.popLsb();
        Bitboard attacks = getAttacks<pieceType, color>(from, position.occupied);
        Bitboard targets = attacks & ~position.getOccupied<color>();
        while (targets) {
            Square to = targets.popLsb();
            moveList.addMove(from, to, piece, static_cast<bool>(position.occupied & to));
        }
    }
}

template <MoveType moveType>
void generateMoves(MoveList& moveList, const Position& position) {

    if (position.sideToMove == Color::White) {
        generatePawnMoves<moveType, Color::White>(moveList, position);
        generateCastlingMoves<moveType, Color::White>(moveList, position);
        generatePieceMoves<moveType, Piece::WhiteKnight>(moveList, position);
        generatePieceMoves<moveType, Piece::WhiteBishop>(moveList, position);
        generatePieceMoves<moveType, Piece::WhiteRook>(moveList, position);
        generatePieceMoves<moveType, Piece::WhiteQueen>(moveList, position);
        generatePieceMoves<moveType, Piece::WhiteKing>(moveList, position);
    }
    else {
        generatePawnMoves<moveType, Color::Black>(moveList, position);
        generateCastlingMoves<moveType, Color::Black>(moveList, position);
        generatePieceMoves<moveType, Piece::BlackKnight>(moveList, position);
        generatePieceMoves<moveType, Piece::BlackBishop>(moveList, position);
        generatePieceMoves<moveType, Piece::BlackRook>(moveList, position);
        generatePieceMoves<moveType, Piece::BlackQueen>(moveList, position);
        generatePieceMoves<moveType, Piece::BlackKing>(moveList, position);
    }
}
