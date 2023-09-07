
#include "attacks.hpp"
#include "universalchessinterface.hpp"
#include "zobrist.hpp"


int main(int argc, char **argv)
{
    initAttacks();
    initZobristKeys();

    UniversalChessInterface uci;
    uci.loop(argc, argv);

    return 0;
}
