#include "perft.hpp"

#include <iostream>

#include "move.hpp"
#include "movegen.hpp"
#include "movelist.hpp"
#include "timeman.hpp"


std::uint64_t perft_driver(Position& pos, const std::uint32_t depth) {
    if (depth == 0)
        return 1;

    MoveList move_list;
    generate_moves<MoveType::AllMoves>(move_list, pos);

    std::uint64_t nodes = 0;

    for (std::uint32_t count = 0; count < move_list.get_size(); ++count) {
        Position next_pos = Position(pos);
        if (!next_pos.make_move(move_list[count]))
            continue;

        nodes += perft_driver(next_pos, depth - 1);
    }

    return nodes;
}

void perft(const Position& pos, const std::uint32_t depth) {
    std::cout << "Perft to depth " << depth << "\n\n";

    TimePoint start_time = get_time();
    MoveList move_list;
    generate_moves<MoveType::AllMoves>(move_list, pos);
    
    std::uint64_t nodes = 0;

    for (std::uint32_t count = 0; count < move_list.get_size(); ++count) {
        Position next_pos = Position(pos);
        if (!next_pos.make_move(move_list[count])) {
            continue;
        }
        

        std::uint64_t old_nodes = nodes;

        nodes += perft_driver(next_pos, depth - 1);

        std::cout << move_list[count] << ": " << nodes - old_nodes << "\n";
    }

    TimePoint end_time = get_time();
    std::cout << "\n\nNodes: " << nodes << std::endl;
    std::cout << "Time: " << end_time - start_time << "ms" << std::endl;
    std::cout << "NPS: " << static_cast<std::uint32_t>(nodes / ((end_time - start_time) / 1000.0)) << std::endl;
}
