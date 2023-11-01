#include "see.hpp"

#include "attacks.hpp"
#include "bitboard.hpp"
#include "color.hpp"
#include "movegen.hpp"
#include "square.hpp"

bool staticExchangeEvaluation(const Position &position, const Move move, std::int32_t threshold) {
    if (move.isPromotion()) return true;

    const Square to = move.getTo();
    const Square from = move.getFrom();

    PieceType capturedPiece = getPieceType(position.pieceAt(to));

    std::int32_t value = seeValue[static_cast<std::uint8_t>(capturedPiece)] - threshold;
    if (value < 0) return false;

    value = value - seeValue[static_cast<std::uint8_t>(getPieceType(move.getPiece()))];
    if (value >= 0) return true;

    const Bitboard whiteOccupied = position.getOccupied<Color::White>();
    const Bitboard blackOccupied = position.getOccupied<Color::Black>();

    Bitboard occupied = (whiteOccupied | blackOccupied) ^ from;
    Bitboard bishopQueenBitboard = position.white.bishops | position.black.bishops | position.white.queens | position.black.queens;
    Bitboard rookQueenBitboard = position.white.rooks | position.black.bishops | position.white.queens | position.black.queens;

    Bitboard attackers = position.getAttackers(to, occupied);
    Color sideToMove = ~position.sideToMove;

    for (;;) {
        attackers &= occupied;

        Bitboard sideToMoveAttackers = attackers & ((sideToMove == Color::White) ? whiteOccupied : blackOccupied);
        Bitboard otherSideAttackers = attackers & ((sideToMove == Color::White) ? blackOccupied : whiteOccupied);
        if (!sideToMoveAttackers) break;

        // find less valuable attackers
        PieceType attacker;
        for (std::uint8_t pieceType = 0; pieceType < 6; pieceType++) {
            attacker = static_cast<PieceType>(pieceType);
            if (sideToMoveAttackers & position.getPieces(sideToMove, attacker)) {
                break;
            }
        }

        sideToMove = ~sideToMove;
        value = - value - seeValue[static_cast<std::uint8_t>(attacker)];

        if (value >= 0) {
            if (attacker == PieceType::King) {
                if (otherSideAttackers != 0) sideToMove = ~sideToMove;
            }
            break;
        }

        occupied ^= Square{(sideToMoveAttackers & position.getPieces(~sideToMove, attacker)).lsb()};
        if (attacker == PieceType::Pawn || attacker == PieceType::Bishop || attacker == PieceType::Queen)
            attackers |= getBishopAttacks(to, occupied) & bishopQueenBitboard;
        if (attacker == PieceType::Rook || attacker == PieceType::Queen)
            attackers |= getRookAttacks(to, occupied) & rookQueenBitboard;
    }

    return sideToMove != getPieceColor(move.getPiece());
}

void testSee(const Position& position) {
    MoveList moveList;
    generateMoves<MoveType::NonQuietMoves>(moveList, position);
    for (std::uint32_t i = 0; i < moveList.getSize(); ++i) {
        std::cout << "Testing move " << moveList[i].move << " : " << (staticExchangeEvaluation(position, moveList[i].move, 0) ? "Good capture" : "Bad capture") << std::endl;
    }
}
