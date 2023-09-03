
#include "attacks.hpp"
#include "perft.hpp"
#include "position.hpp"

#include <iostream>

int main()
{
    initAttacks();

    Position pos;
    pos.loadFromFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    std::cout << pos << std::endl;
    perft(pos, 5);

    return 0;
}
