#include "universalchessinterface.hpp"

#include "bench.hpp"
#include "evaluate.hpp"
#include "movelist.hpp"
#include "movegen.hpp"
#include "perft.hpp"
#include "piece.hpp"
#include "timeman.hpp"

#include <cstdint>
#include <sstream>
#include <cstring>
#include <iostream>

Move UniversalChessInterface::parseMove(std::string moveString) {
    MoveList moveList;
    generateMoves<MoveType::AllMoves>(moveList, game.getCurrentPosition());

    // parse squares
    Square sourceSquare {static_cast<uint8_t>((moveString[1] - '1')), static_cast<uint8_t>((moveString[0] - 'a'))};
    Square targetSquare {static_cast<uint8_t>((moveString[3] - '1')), static_cast<uint8_t>((moveString[2] - 'a'))};

    for (std::uint32_t i = 0; i < moveList.getSize(); i++) {
        Move move = moveList[i];

        if (sourceSquare == move.getFrom() && targetSquare == move.getTo()) {
            Piece promotionPiece = move.getPromotionPiece();

            if (promotionPiece != static_cast<Piece>(0)) {
                if ((promotionPiece ==  Piece::WhiteQueen || promotionPiece == Piece::BlackQueen) && (moveString[4] == 'q' || moveString[4] == 'Q')) return move;
                if ((promotionPiece ==  Piece::WhiteKnight || promotionPiece == Piece::BlackKnight) && (moveString[4] == 'n' || moveString[4] == 'N')) return move;
                if ((promotionPiece ==  Piece::WhiteRook || promotionPiece == Piece::BlackRook) && (moveString[4] == 'r' || moveString[4] == 'R')) return move;
                if ((promotionPiece ==  Piece::WhiteBishop || promotionPiece == Piece::BlackBishop) && (moveString[4] == 'b' || moveString[4] == 'B')) return move;
                continue;
            }
            return move;
        }
    }

    return Move{};
}

void UniversalChessInterface::parsePosition(std::istringstream &ss) {
    game.reset();

    std::string token, fen;
    ss >> token;

    if (token == "startpos") {
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        ss >> token;
    }
    else if (token == "fen") {
        while (ss >> token && token != "moves")
            fen += token + " ";
    }
    else {
        std::cout << "info string error: invalid command" << std::endl;
        return;
    }

    Position position;
    position.loadFromFen(fen);
    game.recordPosition(position);

    if (token == "moves") {
        Move move;
        while (ss >> token && (move = parseMove(token)).isValid()) {
            position.makeMove(move);
            game.recordPosition(position);
        }
    }
}

void UniversalChessInterface::parseGo(std::istringstream &ss) {
    if (!game.isValid()) {
        std::cout << "info string error: position not set" << std::endl;
        return;
    }

    std::string token;

    std::uint32_t depth = maxSearchDepth;
    std::uint32_t movesToGo  = 0;
    TimePoint whiteTime = invalidTimePoint;
    TimePoint blackTime = invalidTimePoint;
    TimePoint whiteIncrement = invalidTimePoint;
    TimePoint blackIncrement = invalidTimePoint;
    TimePoint timePerMove = invalidTimePoint;

    while (ss >> token) {
        if (token == "wtime") ss >> whiteTime;
        else if (token == "btime") ss >> blackTime;
        else if (token == "winc") ss >> whiteIncrement;
        else if (token == "binc") ss >> blackIncrement;
        else if (token == "movestogo") ss >> movesToGo;
        else if (token == "depth") ss >> depth;
        else if (token == "nodes") {}
        else if (token == "movetime") ss >> timePerMove;
        else if (token == "infinite") {}
    }

    searchLimits.depthLimit = depth;
    searchLimits.searchTimeStart = getTime();

    {
        Color sideToMove = game.getSideToMove();
        TimeManagerInitData timeManagerInitData {};
        timeManagerInitData.movesToGo = movesToGo;
        timeManagerInitData.remainingTime = (sideToMove == Color::White) ? whiteTime : blackTime;
        timeManagerInitData.theirRemainingTime = (sideToMove == Color::White) ? blackTime : whiteTime;
        timeManagerInitData.timeIncrement = (sideToMove == Color::White) ? whiteIncrement : blackIncrement;
        timeManagerInitData.theirTimeIncrement = (sideToMove == Color::White) ? blackIncrement : whiteIncrement;
        timeManagerInitData.timeMove = timePerMove;
        computeTimeLimits(timeManagerInitData, searchLimits);
    }

    std::cout << "Search Limits: Depth: " << static_cast<int>(searchLimits.depthLimit) << " Time Limit: " << searchLimits.timeLimit << " Ref Start Time: " << searchLimits.searchTimeStart << std::endl;

    search.startSearch(game, searchLimits);
}

void UniversalChessInterface::parsePerft(std::istringstream &ss) {
    std::string token;

    std::uint32_t depth = maxSearchDepth;
    while (ss >> token) {
        if (token == "depth") { ss >> depth; }
    }

    perft(game.getCurrentPosition(), depth);
}

void UniversalChessInterface::bench() {

    std::uint64_t totalNodes = 0;
    TimePoint startTime = getTime();

    searchLimits.timeLimit = invalidTimePoint;
    searchLimits.searchTimeStart = startTime;
    searchLimits.depthLimit = 5;

    for (const auto& benchFen : benchFens) {
        std::cout << "Current position fen: " << benchFen << std::endl;

        Position position;
        position.loadFromFen(benchFen);

        game.reset();
        game.recordPosition(position);

        ThreadData threadData;
        threadData.searchLimits = searchLimits;
        threadData.game = &game;
        threadData.isMainThread = true;
        threadData.searchStack = {};
        threadData.searchStats = {};
        threadData.searchStack[0].position = game.getCurrentPosition();

        search.searchInternal(threadData);
        totalNodes += threadData.searchStats.quiescenceNodeCounter + threadData.searchStats.negamaxNodeCounter;
    }

    TimePoint elapsedTime = getTime() - startTime;
    std::uint64_t nps = 1000 * totalNodes / elapsedTime;
    std::cout << "===========================\nTotal time (ms) : " << elapsedTime << "\nNodes searched  : " << totalNodes << "\nNodes/second    : " << nps << '\n';
    std::cout << totalNodes << " nodes " << nps << " nps" << std::endl;
}

void UniversalChessInterface::loop(int argc, char **argv) {
    if (argc > 1 && (strncmp(argv[1], "bench", 5) == 0)) {
       bench(); return;
    }

    std::string cmd;

    // main loop
    for(;;) {

        if (!std::getline(std::cin, cmd)) continue;

        std::istringstream ss(cmd);
        std::string token;
        ss >> std::skipws >> token;

        if (token == "quit")            break;
        else if (token == "stop")       search.stopSearch();
        else if (token == "uci")        std::cout << "id name NONAME\nid author Thomas Lemercier\nuciok" << std::endl;
        else if (token == "isready")    std::cout << "readyok\n" << std::endl;
        else if (token == "ucinewgame") {}
        else if (token == "position")   parsePosition(ss);
        else if (token == "go")         parseGo(ss);
        else if (token == "bench")      bench();
        else if (token == "perft")      parsePerft(ss);
        else if (token == "eval")       std::cout << "Evaluation value: " << evaluate(game.getCurrentPosition()) << std::endl;
    }

    search.stopSearch();
}
