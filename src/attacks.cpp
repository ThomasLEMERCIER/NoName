#include "attacks.hpp"

#include <cstdint>
#include <cstring> // std::memset
#include <iostream>

#include "rng.hpp"

Bitboard KnightAttacks[64];
Bitboard KingAttacks[64];
Bitboard PawnAttacks[2][64];
Bitboard RookAttacks[64][4096];
Bitboard BishopAttacks[64][512];

Bitboard RookMasks[64];
Bitboard BishopMasks[64];

Bitboard RookMagics[64];
Bitboard BishopMagics[64];
std::uint8_t RookMagicShifts[64];
std::uint8_t BishopMagicShifts[64];

void init_knight_attacks() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard knight { square };
        Bitboard l1 = (knight >> 1) & Bitboard(0x7f7f7f7f7f7f7f7fULL);
        Bitboard l2 = (knight >> 2) & Bitboard(0x3f3f3f3f3f3f3f3fULL);
        Bitboard r1 = (knight << 1) & Bitboard(0xfefefefefefefefeULL);
        Bitboard r2 = (knight << 2) & Bitboard(0xfcfcfcfcfcfcfcfcULL);
        Bitboard h1 = l1 | r1;
        Bitboard h2 = l2 | r2;
        KnightAttacks[square.index()] = (h1<<16) | (h1>>16) | (h2<<8) | (h2>>8);
    }
}

void init_king_attacks() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard king { square };

        Bitboard attacks = king.east() | king.west();
        king |= attacks;
        attacks |= king.north() | king.south();
        KingAttacks[square.index()] = attacks;        
    }
}

void init_pawn_attacks() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard pawn  { square };
        Bitboard attacks = pawn.east() | pawn.west();
        PawnAttacks[static_cast<std::uint8_t>(Color::White)][square.index()] = attacks.north();
        PawnAttacks[static_cast<std::uint8_t>(Color::Black)][square.index()] = attacks.south();
    }
}

void init_rook_attacks() {
    init_rook_masks();
    init_rook_shifts();
    init_rook_magics();
    init_sliding_attacks(SlidingPiece::Rook);
}

void init_bishop_attacks() {
    init_bishop_masks();
    init_bishop_shifts();
    init_bishop_magics();
    init_sliding_attacks(SlidingPiece::Bishop);
}

void init_attacks() {
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_rook_attacks();
    init_bishop_attacks();
}

void init_rook_masks() {
    for (std::uint8_t sq = 0; sq < 64; sq ++) {
        Square square { sq };

        Bitboard horizontal = Bitboard::RankBitboard(square.rank()) & ~Bitboard::FileBitboard(7) & ~Bitboard::FileBitboard(0);
        Bitboard vertical = Bitboard::FileBitboard(square.file()) & ~Bitboard::RankBitboard(7) & ~Bitboard::RankBitboard(0);

        RookMasks[square.index()] = (horizontal | vertical) & ~Bitboard(square);
    }
}

void init_bishop_masks() {
    for (std::uint8_t sq = 0; sq < 64; sq ++) {
        Square square { sq };

        Bitboard diagonal = Bitboard::DiagonalBitboard(square);
        Bitboard anti_diagonal = Bitboard::AntiDiagonalBitboard(square);

        Bitboard mask = (diagonal | anti_diagonal) & ~Bitboard(square);
        mask &= ~Bitboard::FileBitboard(7) & ~Bitboard::FileBitboard(0);
        mask &= ~Bitboard::RankBitboard(7) & ~Bitboard::RankBitboard(0);

        BishopMasks[square.index()] = mask;
    }
}

void init_rook_shifts() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard mask = RookMasks[square.index()];
        RookMagicShifts[square.index()] = 64 - mask.count();
    }
}

void init_bishop_shifts() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard mask = BishopMasks[square.index()];
        BishopMagicShifts[square.index()] = 64 - mask.count();
    }
}

Bitboard get_rook_attacks_OTF(Square square, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    std::uint8_t rank = square.rank();
    std::uint8_t file = square.file();

    for (std::uint8_t r = rank + 1; r < 8 && (attacks |= Bitboard(r, file)) && !(occupied & Bitboard(r, file)); r++);
    for (std::uint8_t f = file + 1; f < 8 && (attacks |= Bitboard(rank, f)) && !(occupied & Bitboard(rank, f)); f++);
    for (std::uint8_t r = rank - 1; r < 8 && (attacks |= Bitboard(r, file)) && !(occupied & Bitboard(r, file)); r--);
    for (std::uint8_t f = file - 1; f < 8 && (attacks |= Bitboard(rank, f)) && !(occupied & Bitboard(rank, f)); f--);

    return attacks;
}

Bitboard get_bishop_attacks_OTF(Square square, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    std::uint8_t rank = square.rank();
    std::uint8_t file = square.file();

    for (std::uint8_t r = rank + 1u, f = file + 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r++, f++);
    for (std::uint8_t r = rank + 1u, f = file - 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r++, f--);
    for (std::uint8_t r = rank - 1u, f = file + 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r--, f++);
    for (std::uint8_t r = rank - 1u, f = file - 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r--, f--);

    return attacks;
}

void init_rook_magics() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        RookMagics[square.index()] = find_magic(square, SlidingPiece::Rook);
    }
}

void init_bishop_magics() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        BishopMagics[square.index()] = find_magic(square, SlidingPiece::Bishop);
    }
}

Bitboard find_magic(Square square, SlidingPiece piece) {
    Bitboard mask = piece == SlidingPiece::Rook ? RookMasks[square.index()] : BishopMasks[square.index()];

    // we init all possible occupancy variations and their corresponding attacks
    // to avoid having to calculate them later when we search for a magic number
    Bitboard occupancy_variations[4096];
    Bitboard attacks[4096];

    // number of occupancy variations for given mask
    std::uint16_t occupancy_variations_count = 1 << mask.count();

    Bitboard variation = 0ULL;
    int variation_index = 0;
    do {
        occupancy_variations[variation_index] = variation;
        attacks[variation_index] = piece == SlidingPiece::Rook ? get_rook_attacks_OTF(square, variation) : get_bishop_attacks_OTF(square, variation);
        variation = (variation - mask) & mask; // Carry-Rippler trick
        variation_index++;
    } while (variation);

    // now we search for a magic number
    // we try random numbers until we find one that works
    // a magic number works if it maps all occupancy variations to unique attacks
    Bitboard magic;
    Bitboard used_attacks[4096] = {0ULL};
    bool magic_found = false;
    PRNG rng { 0x1234567890abcdefULL };
    std::uint8_t shift = piece == SlidingPiece::Rook ? RookMagicShifts[square.index()] : BishopMagicShifts[square.index()];
    
    for (int tries = 0; !magic_found && tries < 10000000; tries++) {
        magic = rng.next_sparse();
        tries++;

        if (((mask * magic) & Bitboard(0xFF00000000000000ULL)).count() < 6) continue;

        std::memset(used_attacks, 0ULL, sizeof(used_attacks));
        magic_found = true;

        for (variation_index = 0; variation_index < occupancy_variations_count; variation_index++) {
            std::uint16_t magic_index = static_cast<std::uint16_t>((occupancy_variations[variation_index] * magic) >> shift);
            if (used_attacks[magic_index] == 0ULL) {
                used_attacks[magic_index] = attacks[variation_index];
            } else if (used_attacks[magic_index] != attacks[variation_index]) {
                magic_found = false;
                break;
            }
        }
    }

    if (magic_found) return magic;
    std::cout << "Magic not found for square " << square << " and piece " << (piece == SlidingPiece::Rook ? "rook" : "bishop") << std::endl;
    return 0ULL;
}

void init_sliding_attacks(SlidingPiece piece) {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard mask = piece == SlidingPiece::Rook ? RookMasks[square.index()] : BishopMasks[square.index()];
        Bitboard magic = piece == SlidingPiece::Rook ? RookMagics[square.index()] : BishopMagics[square.index()];
        std::uint8_t shift = piece == SlidingPiece::Rook ? RookMagicShifts[square.index()] : BishopMagicShifts[square.index()];

        Bitboard variation = 0ULL;
        do {
            Bitboard attacks = piece == SlidingPiece::Rook ? get_rook_attacks_OTF(square, variation) : get_bishop_attacks_OTF(square, variation);
            std::uint16_t magic_index = static_cast<std::uint16_t>((variation * magic) >> shift);
            if (piece == SlidingPiece::Rook) {
                RookAttacks[square.index()][magic_index] = attacks;
            } else {
                BishopAttacks[square.index()][magic_index] = attacks;
            }
            variation = (variation - mask) & mask; // Carry-Rippler trick
        } while (variation);
    }
}

Bitboard get_knight_attacks(Square square) {
    return KnightAttacks[square.index()];
}

Bitboard get_king_attacks(Square square) {
    return KingAttacks[square.index()];
}

Bitboard get_pawn_attacks(Square square, Color color) {
    return PawnAttacks[static_cast<std::uint8_t>(color)][square.index()];
}

Bitboard get_rook_attacks(Square square, Bitboard occupied) {
    Bitboard mask = RookMasks[square.index()];
    Bitboard magic = RookMagics[square.index()];
    std::uint8_t shift = RookMagicShifts[square.index()];
    std::uint16_t magic_index = static_cast<std::uint16_t>(((occupied & mask) * magic) >> shift);
    return RookAttacks[square.index()][magic_index];
}

Bitboard get_bishop_attacks(Square square, Bitboard occupied) {
    Bitboard mask = BishopMasks[square.index()];
    Bitboard magic = BishopMagics[square.index()];
    std::uint8_t shift = BishopMagicShifts[square.index()];
    std::uint16_t magic_index = static_cast<std::uint16_t>(((occupied & mask) * magic) >> shift);
    return BishopAttacks[square.index()][magic_index];
}

Bitboard get_queen_attacks(Square square, Bitboard occupied) {
    return get_rook_attacks(square, occupied) | get_bishop_attacks(square, occupied);
}
