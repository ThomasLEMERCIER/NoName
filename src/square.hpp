#pragma once

#include <cstdint>
#include <string_view>

enum class Direction {
    North,
    East,
    South,
    West,
    NorthEast,
    SouthEast,
    SouthWest,
    NorthWest
};

class Square {
public:
    static constexpr std::string_view squareNames[64 + 1] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "None"
    };

    enum Value : std::uint8_t {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        None
    };

    uint8_t value;
    constexpr Square(uint8_t val) : value(val) {};
    constexpr Square(uint8_t rank, uint8_t file) : value(rank * 8 + file) {};
    constexpr Square() : value(Square::None) {};

    template<Direction dir>
    constexpr Square shift() const {
        if constexpr(dir == Direction::North)       { return north(); }
        if constexpr(dir == Direction::East)        { return east(); }
        if constexpr(dir == Direction::South)       { return south(); }
        if constexpr(dir == Direction::West)        { return west(); }
        if constexpr(dir == Direction::NorthEast)   { return north().east(); }
        if constexpr(dir == Direction::SouthEast)   { return south().east(); }
        if constexpr(dir == Direction::SouthWest)   { return south().west(); }
        if constexpr(dir == Direction::NorthWest)   { return north().west(); }
    }

    constexpr Square north() const { return Square { static_cast<uint8_t>(value + 8) }; }
    constexpr Square east() const { return Square { static_cast<uint8_t>(value + 1) }; }
    constexpr Square south() const { return Square { static_cast<uint8_t>(value - 8) }; }
    constexpr Square west() const { return Square { static_cast<uint8_t>(value - 1) }; }

    constexpr std::uint8_t index() const { return value; }
    constexpr std::uint8_t flipIndex() const { return value ^ 56; }
    constexpr std::uint8_t rank() const { return value / 8u; }
    constexpr std::uint8_t reverseRank() const { return 7 - value / 8u; }
    constexpr std::uint8_t file() const { return value % 8u; }

    constexpr bool operator==(const Square& rhs) const { return value == rhs.value; }
    constexpr bool operator!=(const Square& rhs) const { return value != rhs.value; }

    friend std::ostream& operator<<(std::ostream& output, const Square& sq) {
        output << squareNames[sq.value];
        return output;
    }
};

constexpr Square rookFromCastling(Square kingTo) {
    if (kingTo == Square::G1) { return Square::H1; }
    if (kingTo == Square::C1) { return Square::A1; }
    if (kingTo == Square::G8) { return Square::H8; }
    if (kingTo == Square::C8) { return Square::A8; }
    return Square::None;
}

constexpr Square rookToCastling(Square kingTo) {
    if (kingTo == Square::G1) { return Square::F1; }
    if (kingTo == Square::C1) { return Square::D1; }
    if (kingTo == Square::G8) { return Square::F8; }
    if (kingTo == Square::C8) { return Square::D8; }
    return Square::None;
}
