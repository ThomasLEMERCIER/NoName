#pragma once

#include "position.hpp"
#include "utils.hpp"

constexpr Score pawnValue = 100;
constexpr Score knightValue = 300;
constexpr Score bishopValue = 300;
constexpr Score rookValue = 500;
constexpr Score queenValue = 900;

Score evaluate(const Position& position);
