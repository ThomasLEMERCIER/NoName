
#include "attacks.hpp"
#include "perft.hpp"
#include "position.hpp"

#include <iostream>

int main()
{
    initAttacks();

    Position pos;
    pos.loadFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    std::cout << pos << std::endl;
    perft(pos, 5);

    return 0;
}
