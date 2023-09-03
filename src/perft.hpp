#pragma once

#include "position.hpp"

std::uint64_t perft_driver(Position& pos, const std::uint32_t depth);
void perft(const Position& pos, const std::uint32_t depth);
