#pragma once

#include "move.hpp"
#include "game.hpp"
#include "search.hpp"

class UniversalChessInterface {
private:
    Game game;

    SearchLimits searchLimits;
    Search search;

    Move parseMove(std::string moveString);
    void parsePosition(std::istringstream& ss);
    void parseGo(std::istringstream& ss);
    void parsePerft(std::istringstream& ss);
    void parseSetOption(std::istringstream& ss);
    void bench();
public:
    void loop(int argc, char* argv[]);

};


