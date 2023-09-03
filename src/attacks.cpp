#include "attacks.hpp"

#include "rng.hpp"

#include <algorithm> // std::fill
#include <iostream>

Bitboard knightAttacks[64];
Bitboard kingAttacks[64];
Bitboard pawnAttacks[2][64];
Bitboard rookAttacks[64][4096];
Bitboard bishopAttacks[64][512];

Bitboard rookMasks[64];
Bitboard bishopMasks[64];

Bitboard rookMagics[64];
Bitboard bishopMagics[64];
std::uint8_t rookMagicShifts[64];
std::uint8_t bishopMagicShifts[64];

void initKnightAttacks() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard knight { square };
        Bitboard l1 = (knight >> 1) & Bitboard(0x7f7f7f7f7f7f7f7fULL);
        Bitboard l2 = (knight >> 2) & Bitboard(0x3f3f3f3f3f3f3f3fULL);
        Bitboard r1 = (knight << 1) & Bitboard(0xfefefefefefefefeULL);
        Bitboard r2 = (knight << 2) & Bitboard(0xfcfcfcfcfcfcfcfcULL);
        Bitboard h1 = l1 | r1;
        Bitboard h2 = l2 | r2;
        knightAttacks[square.index()] = (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
    }
}

void initKingAttacks() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard king { square };

        Bitboard attacks = king.east() | king.west();
        king |= attacks;
        attacks |= king.north() | king.south();
        kingAttacks[square.index()] = attacks;
    }
}

void initPawnAttacks() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard pawn  { square };
        Bitboard attacks = pawn.east() | pawn.west();
        pawnAttacks[static_cast<std::uint8_t>(Color::White)][square.index()] = attacks.north();
        pawnAttacks[static_cast<std::uint8_t>(Color::Black)][square.index()] = attacks.south();
    }
}

void initRookAttacks() {
    initRookMasks();
    initRookShifts();
    initRookMagics();
    initSlidingAttacks(SlidingPiece::Rook);
}

void initBishopAttacks() {
    initBishopMasks();
    initBishopShifts();
    initBishopMagics();
    initSlidingAttacks(SlidingPiece::Bishop);
}

void initAttacks() {
    initKnightAttacks();
    initKingAttacks();
    initPawnAttacks();
    initRookAttacks();
    initBishopAttacks();
}

void initRookMasks() {
    for (std::uint8_t sq = 0; sq < 64; sq ++) {
        Square square { sq };

        Bitboard horizontal = Bitboard::RankBitboard(square.rank()) & ~Bitboard::FileBitboard(7) & ~Bitboard::FileBitboard(0);
        Bitboard vertical = Bitboard::FileBitboard(square.file()) & ~Bitboard::RankBitboard(7) & ~Bitboard::RankBitboard(0);

        rookMasks[square.index()] = (horizontal | vertical) & ~Bitboard(square);
    }
}

void initBishopMasks() {
    for (std::uint8_t sq = 0; sq < 64; sq ++) {
        Square square { sq };

        Bitboard diagonal = Bitboard::DiagonalBitboard(square);
        Bitboard antiDiagonal = Bitboard::AntiDiagonalBitboard(square);

        Bitboard mask = (diagonal | antiDiagonal) & ~Bitboard(square);
        mask &= ~Bitboard::FileBitboard(7) & ~Bitboard::FileBitboard(0);
        mask &= ~Bitboard::RankBitboard(7) & ~Bitboard::RankBitboard(0);

        bishopMasks[square.index()] = mask;
    }
}

void initRookShifts() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard mask = rookMasks[square.index()];
        rookMagicShifts[square.index()] = 64 - mask.count();
    }
}

void initBishopShifts() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard mask = bishopMasks[square.index()];
        bishopMagicShifts[square.index()] = 64 - mask.count();
    }
}

Bitboard getRookAttacksOTF(Square square, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    std::uint8_t rank = square.rank();
    std::uint8_t file = square.file();

    for (std::uint8_t r = rank + 1; r < 8 && (attacks |= Bitboard(r, file)) && !(occupied & Bitboard(r, file)); r++);
    for (std::uint8_t f = file + 1; f < 8 && (attacks |= Bitboard(rank, f)) && !(occupied & Bitboard(rank, f)); f++);
    for (std::uint8_t r = rank - 1; r < 8 && (attacks |= Bitboard(r, file)) && !(occupied & Bitboard(r, file)); r--);
    for (std::uint8_t f = file - 1; f < 8 && (attacks |= Bitboard(rank, f)) && !(occupied & Bitboard(rank, f)); f--);

    return attacks;
}

Bitboard getBishopAttacksOTF(Square square, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    std::uint8_t rank = square.rank();
    std::uint8_t file = square.file();

    for (std::uint8_t r = rank + 1u, f = file + 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r++, f++);
    for (std::uint8_t r = rank + 1u, f = file - 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r++, f--);
    for (std::uint8_t r = rank - 1u, f = file + 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r--, f++);
    for (std::uint8_t r = rank - 1u, f = file - 1u; r < 8u && f < 8u && (attacks |= Bitboard(r, f)) && !(occupied & Bitboard(r, f)); r--, f--);

    return attacks;
}

void initRookMagics() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        rookMagics[square.index()] = findMagic(square, SlidingPiece::Rook);
    }
}

void initBishopMagics() {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        bishopMagics[square.index()] = findMagic(square, SlidingPiece::Bishop);
    }
}

Bitboard findMagic(Square square, SlidingPiece piece) {
    Bitboard mask = piece == SlidingPiece::Rook ? rookMasks[square.index()] : bishopMasks[square.index()];

    // we initialize all possible occupancy variations and their corresponding attacks
    // to avoid having to calculate them later when we search for a magic number
    Bitboard occupancyVariations[4096];
    Bitboard attacks[4096];

    // number of occupancy variations for given mask
    std::uint16_t occupancyVariationsCount = 1 << mask.count();

    Bitboard variation = 0ULL;
    int variationIndex = 0;
    do {
        occupancyVariations[variationIndex] = variation;
        attacks[variationIndex] = piece == SlidingPiece::Rook ? getRookAttacksOTF(square, variation) : getBishopAttacksOTF(
                square, variation);
        variation = (variation - mask) & mask; // Carry-Rippler trick
        variationIndex++;
    } while (variation);

    // now we search for a magic number
    // we try random numbers until we find one that works
    // a magic number works if it maps all occupancy variations to unique attacks
    Bitboard magic;
    Bitboard usedAttacks[4096] = {0ULL};
    bool magicFound = false;
    PRNG rng { 0x1234567890abcdefULL };
    std::uint8_t shift = piece == SlidingPiece::Rook ? rookMagicShifts[square.index()] : bishopMagicShifts[square.index()];
    
    for (int tries = 0; !magicFound && tries < 10000000; tries++) {
        magic = rng.nextSparse();
        tries++;

        if (((mask * magic) & Bitboard(0xFF00000000000000ULL)).count() < 6) continue;

        std::fill(usedAttacks, usedAttacks + occupancyVariationsCount, 0ULL);
        magicFound = true;

        for (variationIndex = 0; variationIndex < occupancyVariationsCount; variationIndex++) {
            std::uint16_t magicIndex = static_cast<std::uint16_t>((occupancyVariations[variationIndex] * magic) >> shift);
            if (usedAttacks[magicIndex] == 0ULL) {
                usedAttacks[magicIndex] = attacks[variationIndex];
            } else if (usedAttacks[magicIndex] != attacks[variationIndex]) {
                magicFound = false;
                break;
            }
        }
    }

    if (magicFound) return magic;
    std::cout << "Magic not found for square " << square << " and piece " << (piece == SlidingPiece::Rook ? "rook" : "bishop") << std::endl;
    return 0ULL;
}

void initSlidingAttacks(SlidingPiece piece) {
    for (std::uint8_t sq = 0; sq < 64; sq++) {
        Square square { sq };
        Bitboard mask = piece == SlidingPiece::Rook ? rookMasks[square.index()] : bishopMasks[square.index()];
        Bitboard magic = piece == SlidingPiece::Rook ? rookMagics[square.index()] : bishopMagics[square.index()];
        std::uint8_t shift = piece == SlidingPiece::Rook ? rookMagicShifts[square.index()] : bishopMagicShifts[square.index()];

        Bitboard variation = 0ULL;
        do {
            Bitboard attacks = piece == SlidingPiece::Rook ? getRookAttacksOTF(square, variation) : getBishopAttacksOTF(
                    square, variation);
            std::uint16_t magicIndex = static_cast<std::uint16_t>((variation * magic) >> shift);
            if (piece == SlidingPiece::Rook) {
                rookAttacks[square.index()][magicIndex] = attacks;
            } else {
                bishopAttacks[square.index()][magicIndex] = attacks;
            }
            variation = (variation - mask) & mask; // Carry-Rippler trick
        } while (variation);
    }
}

Bitboard getKnightAttacks(Square square) {
    return knightAttacks[square.index()];
}

Bitboard getKingAttacks(Square square) {
    return kingAttacks[square.index()];
}

Bitboard getPawnAttacks(Square square, Color color) {
    return pawnAttacks[static_cast<std::uint8_t>(color)][square.index()];
}

Bitboard getRookAttacks(Square square, Bitboard occupied) {
    Bitboard mask = rookMasks[square.index()];
    Bitboard magic = rookMagics[square.index()];
    std::uint8_t shift = rookMagicShifts[square.index()];
    std::uint16_t magicIndex = static_cast<std::uint16_t>(((occupied & mask) * magic) >> shift);
    return rookAttacks[square.index()][magicIndex];
}

Bitboard getBishopAttacks(Square square, Bitboard occupied) {
    Bitboard mask = bishopMasks[square.index()];
    Bitboard magic = bishopMagics[square.index()];
    std::uint8_t shift = bishopMagicShifts[square.index()];
    std::uint16_t magicIndex = static_cast<std::uint16_t>(((occupied & mask) * magic) >> shift);
    return bishopAttacks[square.index()][magicIndex];
}

Bitboard getQueenAttacks(Square square, Bitboard occupied) {
    return getRookAttacks(square, occupied) | getBishopAttacks(square, occupied);
}
