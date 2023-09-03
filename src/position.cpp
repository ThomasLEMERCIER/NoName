#include "position.hpp"

#include <sstream>

#include "attacks.hpp"

// forwards instantiation of template function
template bool Position::is_square_attacked_by<Color::White>(const Square square) const;
template bool Position::is_square_attacked_by<Color::Black>(const Square square) const;

void Position::load_from_fen(const std::string& fen) {
    std::istringstream fen_stream { fen };

    fen_stream >> std::noskipws;
    unsigned char token;


    std::uint8_t rank = 7, file = 0;

    while ((fen_stream  >> token) && token != ' ') {
        if (token == '/') {
            rank--;
            file = 0;
        }
        else if (isdigit(token)) {
            file += (token - '0');
        }
        else {
            const Piece piece = PieceChars.at(token);
            const PieceType piece_type = get_piece_type(piece); 
            const Square sq {rank, file};
            const Color color = (piece < Piece::BlackPawn) ? Color::White : Color::Black;

            set_piece(color, piece_type, sq);
            file++;
        }
    }

    fen_stream >> token;
    side_to_move = (token == 'w') ? Color::White : Color::Black;

    fen_stream >> token;
    while ((fen_stream >> token) && token != ' ') {
        switch (token)
        {
            case 'K': castling_rights |= CastlingRight::WhiteKingSide; break;
            case 'Q': castling_rights |= CastlingRight::WhiteQueenSide; break;
            case 'k': castling_rights |= CastlingRight::BlackKingSide; break;
            case 'q': castling_rights |= CastlingRight::BlackQueenSide; break;
            default: break;
        }
    }

    fen_stream >> token;
    if (token == '-') {
        en_passant_square = Square::None;
    }
    else {
        file = token - 'a';
        fen_stream >> token;
        rank = (token - '1');
        en_passant_square = Square { rank, file }; 
    }
}

void Position::set_piece(const Color color, const PieceType piece, const Square square) {
    Bitboard mask { square };
    SidePosition& side = (color == Color::White) ? white : black;

    side[piece] |= mask;
    side.occupied |= mask;
    occupied |= mask;
}

void Position::remove_piece(const Color color, const PieceType piece, const Square square) {
    Bitboard mask { square };
    SidePosition& side = (color == Color::White) ? white : black;

    side[piece] ^= mask;
    side.occupied ^= mask;
    occupied ^= mask;
}

Piece Position::piece_at(const Square square) const {
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
bool Position::is_square_attacked_by(const Square square) const {
    if constexpr (color == Color::White) {
        if (white.pawns & get_pawn_attacks(square, Color::Black))   { return true; }
        if (white.knights & get_knight_attacks(square))             { return true; }
        if (white.bishops & get_bishop_attacks(square, occupied))   { return true; }
        if (white.rooks & get_rook_attacks(square, occupied))       { return true; }
        if (white.queens & get_queen_attacks(square, occupied))     { return true; }
        if (white.king & get_king_attacks(square))                  { return true; }
    }
    else {
        if (black.pawns & get_pawn_attacks(square, Color::White))   { return true; }
        if (black.knights & get_knight_attacks(square))             { return true; }
        if (black.bishops & get_bishop_attacks(square, occupied))   { return true; }
        if (black.rooks & get_rook_attacks(square, occupied))       { return true; }
        if (black.queens & get_queen_attacks(square, occupied))     { return true; }
        if (black.king & get_king_attacks(square))                  { return true; }
    }

    return false;
}

bool Position::is_in_check(const Color color) const {
    if (color == Color::White)  { return is_square_attacked_by<Color::Black>(white.king.Lsb()); }
    else                        { return is_square_attacked_by<Color::White>(black.king.Lsb()); }
}

bool Position::make_move(const Move move) {
    const bool capture = move.is_capture();
    const bool double_push = move.is_double_push();
    const bool enpassant = move.is_enpassant();
    const bool castling = move.is_castling();

    const Square from = move.get_from();
    const Square to = move.get_to();
    const Piece piece = move.get_piece();
    const PieceType piece_type = get_piece_type(piece);
    const Piece promotion_piece = move.get_promotion_piece();
    const PieceType promotion_piece_type = get_piece_type(promotion_piece);
    const Piece captured_piece = enpassant ? Piece::WhitePawn : piece_at(to);
    const PieceType captured_piece_type = get_piece_type(captured_piece);

    const std::uint8_t opponent_enpassant_rank = (side_to_move == Color::White) ? 4 : 3;
    const std::uint8_t enpassant_rank = (side_to_move == Color::White) ? 2 : 5;

    if (castling) {
        Square rook_from = RookFromCastling(to);
        Square rook_to = RookToCastling(to);

        remove_piece(side_to_move, PieceType::Rook, rook_from);
        set_piece(side_to_move, PieceType::Rook, rook_to);
    }
    else if (capture) {
        Square captured_square = enpassant ? Square(opponent_enpassant_rank, to.file()) : to;
        remove_piece(~side_to_move, captured_piece_type, captured_square);
    }

    remove_piece(side_to_move, piece_type, from);
    set_piece(side_to_move, piece_type, to);

    if (en_passant_square != Square::None) {
        en_passant_square = Square::None;
    }

    if (piece_type == PieceType::Pawn) {
        if (double_push) {
            en_passant_square = Square(enpassant_rank, to.file());
        }
        else if (promotion_piece != static_cast<Piece>(0)) {
            remove_piece(side_to_move, PieceType::Pawn, to);
            set_piece(side_to_move, promotion_piece_type, to);
        }
    }

    // if (castling_rights != CastlingRight::None) {
        castling_rights &= CastlingRightUpdate[from.index()];
        castling_rights &= CastlingRightUpdate[to.index()];
    // }

    Color prev_side_to_move = side_to_move;
    side_to_move = ~side_to_move;

    return !is_in_check(prev_side_to_move);
}

std::ostream& operator<<(std::ostream& output, const Position& pos) {
    for (std::uint8_t rank = 8; rank-- > 0;) {
        output << static_cast<int>(rank + 1) << " |";
        for (std::uint8_t file = 0; file < 8; ++file) {
            const Square sq = Square { rank, file };
            const Piece piece = pos.piece_at(sq);

            output << " " << PieceNames[static_cast<std::uint8_t>(piece)];
        }
        output << "\n";
    }

    output << "   ----------------\n";
    output << "    a b c d e f g h\n";

    output << "\n\n    Side to move: " << ((pos.side_to_move == Color::White) ? "White" : "Black");
    output << "\n    Castling rights: ";
    if ((pos.castling_rights & CastlingRight::WhiteKingSide) != CastlingRight::None)    { output << "K"; }
    if ((pos.castling_rights & CastlingRight::WhiteQueenSide) != CastlingRight::None)   { output << "Q"; }
    if ((pos.castling_rights & CastlingRight::BlackKingSide) != CastlingRight::None)    { output << "k"; }
    if ((pos.castling_rights & CastlingRight::BlackQueenSide) != CastlingRight::None)   { output << "q"; }
    output << "\n    En passant square: " << pos.en_passant_square << std::endl;

    // output << "\n\n    White occupied: \n" << pos.white.occupied;
    // output << "\n\n    Black occupied: \n" << pos.black.occupied;
    // output << "\n\n    All occupied: \n" << pos.occupied;

    return output;
}
