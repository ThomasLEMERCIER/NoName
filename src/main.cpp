#include <iostream>
#include <bitset>

#include "bitboard.hpp"
#include "square.hpp"
#include "position.hpp"
#include "attacks.hpp"
#include "rng.hpp"
#include "movelist.hpp"
#include "movegen.hpp"
#include "perft.hpp"

int main()
{
    init_attacks();

    Position pos;

    pos.load_from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

    // MoveList move_list;
    
    // move_list.clear();
    // generate_moves<MoveType::AllMoves>(move_list, pos);
    // pos.make_move(move_list[47]);

    // move_list.clear();
    // generate_moves<MoveType::AllMoves>(move_list, pos);
    // pos.make_move(move_list[0]);

    // move_list.clear();
    // generate_moves<MoveType::AllMoves>(move_list, pos);
    // pos.make_move(move_list[0]);

    // move_list.clear();
    // generate_moves<MoveType::AllMoves>(move_list, pos);
    // pos.make_move(move_list[6]);

    // move_list.clear();
    // generate_moves<MoveType::AllMoves>(move_list, pos);
    // pos.make_move(move_list[0]);


    std::cout << pos << std::endl;
    perft(pos, 7);

    return 0;
}
