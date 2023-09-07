#include "position.hpp"

#include "attacks.hpp"
#include "zobrist.hpp"

#include <sstream>

// forwards instantiation of template function
template bool Position::isSquareAttackedBy<Color::White>(const Square square) const;
template bool Position::isSquareAttackedBy<Color::Black>(const Square square) const;

void Position::loadFromFen(const std::string& fen) {
    *this = Position();

    std::istringstream fenStream {fen };

    fenStream >> std::noskipws;
    char token;
    std::uint8_t rank = 7, file = 0;

    while ((fenStream >> token) && token != ' ') {
        if (token == '/') {
            rank--;
            file = 0;
        }
        else if (isdigit(token)) {
            file += (token - '0');
        }
        else {
            const Piece piece = pieceChars.at(token);
            const PieceType pieceType = getPieceType(piece);
            const Square sq {rank, file};
            const Color color = (piece < Piece::BlackPawn) ? Color::White : Color::Black;

            setPiece(color, pieceType, sq);
            file++;
        }
    }

    fenStream >> token;
    sideToMove = (token == 'w') ? Color::White : Color::Black;

    fenStream >> token;
    while ((fenStream >> token) && token != ' ') {
        switch (token)
        {
            case 'K': castlingRights |= CastlingRight::WhiteKingSide; break;
            case 'Q': castlingRights |= CastlingRight::WhiteQueenSide; break;
            case 'k': castlingRights |= CastlingRight::BlackKingSide; break;
            case 'q': castlingRights |= CastlingRight::BlackQueenSide; break;
            default: break;
        }
    }

    fenStream >> token;
    if (token == '-') {
        enPassantSquare = Square::None;
    }
    else {
        file = token - 'a';
        fenStream >> token;
        rank = (token - '1');
        enPassantSquare = Square {rank, file };
    }

    hash = computeHash();
}

void Position::setPiece(const Color color, const PieceType piece, const Square square) {
    Bitboard mask { square };
    SidePosition& side = (color == Color::White) ? white : black;

    side[piece] |= mask;
    side.occupied |= mask;
    occupied |= mask;
    hash ^= getPieceSquareHash(color, piece, square);
}

void Position::removePiece(const Color color, const PieceType piece, const Square square) {
    Bitboard mask { square };
    SidePosition& side = (color == Color::White) ? white : black;

    side[piece] ^= mask;
    side.occupied ^= mask;
    occupied ^= mask;
    hash ^= getPieceSquareHash(color, piece, square);
}

Piece Position::pieceAt(const Square square) const {
    Bitboard mask { square };

    // check if square is occupied by white piece
    if (white.pawns & mask) { return Piece::WhitePawn; }
    if (white.knights & mask) { return Piece::WhiteKnight; }
    if (white.bishops & mask) { return Piece::WhiteBishop; }
    if (white.rooks & mask) { return Piece::WhiteRook; }
    if (white.queens & mask) { return Piece::WhiteQueen; }
    if (white.king & mask) { return Piece::WhiteKing; }

    // check if square is occupied by black pieces
    if (black.pawns & mask) { return Piece::BlackPawn; }
    if (black.knights & mask) { return Piece::BlackKnight; }
    if (black.bishops & mask) { return Piece::BlackBishop; }  
    if (black.rooks & mask) { return Piece::BlackRook; }
    if (black.queens & mask) { return Piece::BlackQueen; }
    if (black.king & mask) { return Piece::BlackKing; }

    // if square is not occupied by any piece, return Piece::None
    return Piece::None;
} 

template<Color color>
bool Position::isSquareAttackedBy(const Square square) const {
    if constexpr (color == Color::White) {
        if (white.pawns & getPawnAttacks(square, Color::Black))   { return true; }
        if (white.knights & getKnightAttacks(square))             { return true; }
        if (white.bishops & getBishopAttacks(square, occupied))   { return true; }
        if (white.rooks & getRookAttacks(square, occupied))       { return true; }
        if (white.queens & getQueenAttacks(square, occupied))     { return true; }
        if (white.king & getKingAttacks(square))                  { return true; }
    }
    else {
        if (black.pawns & getPawnAttacks(square, Color::White))   { return true; }
        if (black.knights & getKnightAttacks(square))             { return true; }
        if (black.bishops & getBishopAttacks(square, occupied))   { return true; }
        if (black.rooks & getRookAttacks(square, occupied))       { return true; }
        if (black.queens & getQueenAttacks(square, occupied))     { return true; }
        if (black.king & getKingAttacks(square))                  { return true; }
    }

    return false;
}

bool Position::isInCheck(const Color color) const {
    if (color == Color::White)  { return isSquareAttackedBy<Color::Black>(white.king.lsb()); }
    else                        { return isSquareAttackedBy<Color::White>(black.king.lsb()); }
}

bool Position::makeMove(const Move move) {
    const bool capture = move.isCapture();
    const bool doublePush = move.isDoublePush();
    const bool enpassant = move.isEnpassant();
    const bool castling = move.isCastling();

    const Square from = move.getFrom();
    const Square to = move.getTo();
    const Piece piece = move.getPiece();
    const PieceType pieceType = getPieceType(piece);
    const Piece promotionPiece = move.getPromotionPiece();
    const PieceType promotionPieceType = getPieceType(promotionPiece);
    const Piece capturedPiece = enpassant ? Piece::WhitePawn : pieceAt(to);
    const PieceType capturedPieceType = getPieceType(capturedPiece);

    const std::uint8_t opponentEnpassantRank = (sideToMove == Color::White) ? 4 : 3;
    const std::uint8_t enpassantRank = (sideToMove == Color::White) ? 2 : 5;

    if (castling) {
        Square rookFrom = rookFromCastling(to);
        Square rookTo = rookToCastling(to);

        removePiece(sideToMove, PieceType::Rook, rookFrom);
        setPiece(sideToMove, PieceType::Rook, rookTo);
    }
    else if (capture) {
        Square capturedSquare = enpassant ? Square(opponentEnpassantRank, to.file()) : to;
        removePiece(~sideToMove, capturedPieceType, capturedSquare);
    }

    removePiece(sideToMove, pieceType, from);
    setPiece(sideToMove, pieceType, to);

    if (enPassantSquare != Square::None) {
        hash ^= enPassantFileZobristHash[enPassantSquare.file()];
        enPassantSquare = Square::None;
    }

    if (pieceType == PieceType::Pawn) {
        if (doublePush) {
            enPassantSquare = Square(enpassantRank, to.file());
            hash ^= enPassantFileZobristHash[enPassantSquare.file()];
        }
        else if (promotionPiece != static_cast<Piece>(0)) {
            removePiece(sideToMove, PieceType::Pawn, to);
            setPiece(sideToMove, promotionPieceType, to);
        }
    }

    if (castlingRights != CastlingRight::None) {
        hash ^= castlingRightZobristHash[static_cast<std::uint8_t>(castlingRights)];
        castlingRights &= castlingRightUpdate[from.index()];
        castlingRights &= castlingRightUpdate[to.index()];
        hash ^= castlingRightZobristHash[static_cast<std::uint8_t>(castlingRights)];
    }

    Color prevSideToMove = sideToMove;
    sideToMove = ~sideToMove;
    hash ^= colorZobristHash;

    return !isInCheck(prevSideToMove);
}

std::uint64_t Position::computeHash() {
    std::uint64_t hashValue = (sideToMove == Color::Black) ? colorZobristHash : 0ULL;

    for (std::uint8_t pieceType = 0; pieceType < 6; pieceType++) {
        Bitboard pieceBitboard = white[static_cast<PieceType>(pieceType)];
        while (pieceBitboard) {
            Square square = pieceBitboard.popLsb();
            hashValue ^= getPieceSquareHash(Color::White, static_cast<PieceType>(pieceType), square);
        }
    }

    for (std::uint8_t pieceType = 0; pieceType < 6; pieceType++) {
        Bitboard pieceBitboard = black[static_cast<PieceType>(pieceType)];
        while (pieceBitboard) {
            Square square = pieceBitboard.popLsb();
            hashValue ^= getPieceSquareHash(Color::Black, static_cast<PieceType>(pieceType), square);
        }
    }

    if (enPassantSquare != Square::None) {
        hashValue ^= enPassantFileZobristHash[enPassantSquare.file()];
    }

    hashValue ^= castlingRightZobristHash[static_cast<std::uint8_t>(castlingRights)];

    return hashValue;
}

std::ostream& operator<<(std::ostream& output, const Position& pos) {
    for (std::uint8_t rank = 8; rank-- > 0;) {
        output << static_cast<int>(rank + 1) << " |";
        for (std::uint8_t file = 0; file < 8; ++file) {
            const Square sq = Square { rank, file };
            const Piece piece = pos.pieceAt(sq);

            output << " " << pieceNames[static_cast<std::uint8_t>(piece)];
        }
        output << "\n";
    }

    output << "   ----------------\n";
    output << "    a b c d e f g h\n";

    output << "\n\n    Side to move: " << ((pos.sideToMove == Color::White) ? "White" : "Black");
    output << "\n    Castling rights: ";
    if ((pos.castlingRights & CastlingRight::WhiteKingSide) != CastlingRight::None)    { output << "K"; }
    if ((pos.castlingRights & CastlingRight::WhiteQueenSide) != CastlingRight::None)   { output << "Q"; }
    if ((pos.castlingRights & CastlingRight::BlackKingSide) != CastlingRight::None)    { output << "k"; }
    if ((pos.castlingRights & CastlingRight::BlackQueenSide) != CastlingRight::None)   { output << "q"; }
    if (pos.castlingRights == CastlingRight::None)   { output << " None"; }
    output << "\n    En passant square: " << pos.enPassantSquare << std::endl;
    output << "\n\n    Hash Value: 0x" << std::hex << pos.hash << std::dec << std::endl;

    // output << "\n\n    White occupied: \n" << pos.white.occupied;
    // output << "\n\n    Black occupied: \n" << pos.black.occupied;
    // output << "\n\n    All occupied: \n" << pos.occupied;

    return output;
}

