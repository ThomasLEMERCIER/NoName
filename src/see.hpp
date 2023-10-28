#pragma once

#include "move.hpp"
#include "position.hpp"

constexpr std::int32_t seeValue[6] = { 100, 300, 300, 500, 900, INT32_MAX };

bool staticExchangeEvaluation(const Position& position, const Move move, std::int32_t threshold);
void testSee(const Position& position);
