
#include "attacks.hpp"
#include "evaluate.hpp"
#include "universalchessinterface.hpp"
#include "search.hpp"
#include "zobrist.hpp"


int main(int argc, char **argv)
{
    initAttacks();
    initZobristKeys();
    initSearchParameters();
    initEvaluationParameters();

    UniversalChessInterface uci;
    uci.loop(argc, argv);

    return 0;
}
