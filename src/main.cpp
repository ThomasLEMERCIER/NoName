#ifdef TUNER
#include "tuner.hpp"
#endif

#include "attacks.hpp"
#include "universalchessinterface.hpp"
#include "zobrist.hpp"


int main(int argc, char **argv)
{
    initAttacks();
    initZobristKeys();

#ifdef TUNER
    startTuner();
#endif

//    UniversalChessInterface uci;
//    uci.loop(argc, argv);

    return 0;
}
