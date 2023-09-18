#include "game.hpp"

#include <algorithm>

bool Game::checkRepetition(std::uint64_t hash) const {
    return std::ranges::any_of(positionHistory.rbegin(), positionHistory.rend(), [&](auto& previousHash) { return previousHash == hash;});
}

void Game::recordPosition(Position& position) {
    currentPosition = position;
    positionHistory.push_back(position.hash);
    valid = true;
}

void Game::reset() {
    valid = false;
    positionHistory.clear();
}
