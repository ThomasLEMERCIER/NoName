#pragma once

#include <cstdint>
#include <iostream>

#include "square.hpp"
#include "utils.hpp"

class Bitboard
{
private:
    std::uint64_t value;

public:
    static constexpr Bitboard FileBitboard(std::uint8_t file) { return Bitboard { 0x0101010101010101ULL << file }; }
    static constexpr Bitboard RankBitboard(std::uint8_t rank) { return Bitboard { 0xffULL << (rank * 8) }; }
    static constexpr Bitboard DiagonalBitboard(Square square) { Bitboard main_diag {0x8040201008040201ULL};
                                                                int diag  = static_cast<int>(square.file()) - static_cast<int>(square.rank());
                                                                return diag >= 0 ? main_diag >> diag * 8 : main_diag << -diag * 8; }
    static constexpr Bitboard AntiDiagonalBitboard(Square square) { Bitboard main_diag {0x0102040810204080ULL};
                                                                    int diag  = 7 - static_cast<int>(square.file()) - static_cast<int>(square.rank());
                                                                    return diag >= 0 ? main_diag >> diag * 8 : main_diag << -diag * 8; }
    
    constexpr Bitboard(std::uint64_t val = 0ULL) : value(val) {};
    constexpr Bitboard(Square square) : value(1ULL << square.index()) {};
    constexpr Bitboard(std::uint8_t rank, std::uint8_t file) : value(1ULL << (rank * 8 + file)) {};

    constexpr Bitboard operator&(const Bitboard& rhs) const { return Bitboard { value & rhs.value }; }
    constexpr Bitboard operator|(const Bitboard& rhs) const { return Bitboard { value | rhs.value }; }
    constexpr Bitboard operator^(const Bitboard& rhs) const { return Bitboard { value ^ rhs.value }; }
    constexpr Bitboard operator~() const { return Bitboard { ~value }; }
    constexpr Bitboard operator<<(std::uint8_t shift) const { return Bitboard { value << shift }; }
    constexpr Bitboard operator>>(std::uint8_t shift) const { return Bitboard { value >> shift }; }

    constexpr Bitboard operator-(const Bitboard& rhs) const { return Bitboard { value - rhs.value }; }
    constexpr Bitboard operator*(const Bitboard& rhs) const { return Bitboard { value * rhs.value }; }

    constexpr Bitboard& operator&=(const Bitboard& rhs) { value &= rhs.value; return *this; }
    constexpr Bitboard& operator|=(const Bitboard& rhs) { value |= rhs.value; return *this; }
    constexpr Bitboard& operator^=(const Bitboard& rhs) { value ^= rhs.value; return *this; }

    template<Direction dir>
    constexpr Bitboard shift() {
        if constexpr(dir == Direction::North)       { return north(); }
        if constexpr(dir == Direction::East)        { return east(); }
        if constexpr(dir == Direction::South)       { return south(); }
        if constexpr(dir == Direction::West)        { return west(); }
        if constexpr(dir == Direction::NorthEast)   { return north().east(); }
        if constexpr(dir == Direction::SouthEast)   { return south().east(); }
        if constexpr(dir == Direction::SouthWest)   { return south().west(); }
        if constexpr(dir == Direction::NorthWest)   { return north().west(); }
    }

    constexpr Bitboard north() const { return Bitboard { value << 8u }; }
    constexpr Bitboard east() const { return Bitboard { value << 1u } & ~FileBitboard(0); }
    constexpr Bitboard south() const { return Bitboard { value >> 8u }; }
    constexpr Bitboard west() const { return Bitboard { value >> 1u } & ~FileBitboard(7); }

    constexpr std::uint8_t count() const { return PopCount(value); }
    constexpr std::uint8_t PopLsb() { std::uint8_t index = GetLsbIndex(value); value &= value - 1; return index; }
    constexpr std::uint8_t Lsb() const { return GetLsbIndex(value); }

    constexpr bool operator==(const Bitboard& rhs) const { return value == rhs.value; }
    constexpr bool operator!=(const Bitboard& rhs) const { return value != rhs.value; }

    explicit constexpr operator bool() const { return value != 0ULL; }
    explicit constexpr operator std::uint16_t() const { return static_cast<std::uint16_t>(value); }

    friend std::ostream& operator<<(std::ostream& output, const Bitboard& bb); // output operator
};
