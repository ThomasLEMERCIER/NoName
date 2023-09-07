#pragma once

#include "move.hpp"
#include "position.hpp"
#include "search.hpp"

class UniversalChessInterface {
private:
    bool positionSet {false};
    Position position {};

    SearchLimits searchLimits;
    Search search;

    Move parseMove(std::string moveString);
    void parsePosition(std::istringstream& ss);
    void parseGo(std::istringstream& ss);
    void parsePerft(std::istringstream& ss);
    void bench();
public:
    void loop(int argc, char* argv[]);

};


