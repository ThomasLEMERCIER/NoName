#include "bitboard.hpp"

std::ostream& operator<<(std::ostream& output, const Bitboard& bb) {
    for (std::uint8_t rank = 8; rank-- > 0;) {
        output << static_cast<int>(rank + 1) << " |";
        for (std::uint8_t file = 0; file < 8; ++file) {
            output << " " << ((bb.value >> (rank * 8 + file)) & 1ULL);
        }
        output << "\n";
    }

    output << "   ----------------\n";
    output << "    a b c d e f g h\n";
    output << "\n\n    Value: 0x" << std::hex << bb.value << std::dec << std::endl;
    return output;
}
