#include "tuner.hpp"

#include "position.hpp"

#include <iostream>
#include <fstream>
#include <cmath>

void startTuner() {

    std::vector<Entry> entries;
    double k, error;
    WeightVector weights;

    std::cout << "Starting tuning\n";
    std::cout << "Number of terms to tune: " << nTerms;
    std::cout << "\nNumber of epochs to do: " << nEpochs;
    std::cout << "\nLearning rate set to: " << learningRate << std::endl;

    if (!initEntries(entries)) {
        std::cout << "Error while reading data file" << std::endl;
        return;
    }

    k = computeOptimalK(entries);
    std::cout << "\nOptimal K found at: " << k << std::endl;

    for (std::uint32_t epoch = 0; epoch < nEpochs; ++epoch) {
        std::cout << "Starting epoch " << epoch+1 << "/" << nEpochs << std::endl;
        for (std::uint32_t batchIndex = 0; batchIndex < entries.size() / batchSize; ++batchIndex) {
            GradientVector gradients = {{0}};
            computeBatchedGradient(entries, k, weights, batchIndex, gradients);

            for (std::uint32_t i = 0; i < nTerms; i++) {
                weights[i][mg] += (k / batchSize) * learningRate * gradients[i][mg];
                weights[i][eg] += (k / batchSize) * learningRate * gradients[i][eg];
            }
        }

        error = computeTotalError(entries, weights, k);
        std::cout << "Epoch error: " << error << std::endl;
    }

    error = computeTotalError(entries, weights, k);
    std::cout << "Epoch error: " << error << std::endl;
};

bool initEntries(std::vector<Entry>& entries) {

    std::ifstream fenFile{ "fens" };
    if (!fenFile) return false;

    // Parsing variables
    std::string line;
    char c;
    double result;
    std::string fen;

    Position position;

    while (std::getline(fenFile, line))
    {
        std::istringstream iss(line);
        iss >> std::noskipws;

        iss >> c >> result >> c >> c;
        std::getline(iss, fen);

        position.loadFromFen(fen);
        Score eval = evaluate(position);

        Entry entry;
        entry.result = result;
        entry.eval = (position.sideToMove == Color::White) ? eval : -eval;

        for (std::uint32_t indexCoefficient = 0; indexCoefficient < nTerms; ++indexCoefficient) {
            EvalTrace trace = *((EvalTrace*)&evalVector + indexCoefficient);
            if (trace.white != 0 || trace.black != 0) {
                entry.coefs.emplace_back(indexCoefficient, trace.white, trace.black);
            }
        }

        entry.phase = knightPhaseValue * (evalVector.knightCount.white + evalVector.knightCount.black)
                    + bishopPhaseValue * (evalVector.bishopCount.white + evalVector.bishopCount.black)
                    + rookPhaseValue * (evalVector.rookCount.white + evalVector.rookCount.black)
                    + queenPhaseValue * (evalVector.queenCount.white + evalVector.queenCount.black);

        entries.push_back(entry);
    }

    return true;
}

double computeOptimalK(std::vector<Entry> &entries) {
    double k = 0;

    for (std::uint32_t epoch = 0; epoch < kPrecisionEpochs; ++epoch) {
        double derivative = computeDerivativeOverK(entries, k);
//        double error = computeTotalError(entries, k);
        k -=  learningRateK * derivative;

//        std::cout << "(" << epoch+1 << "/" << kPrecisionEpochs << "): " << error << std::endl;
    }
    return k;
}

double sigmoid(double x, double k) {
    return 1. / (1. + std::exp(- x * k));
}

double computeDerivativeOverK(std::vector<Entry> &entries,  double k) {
    double derivative = 0;

    for (const auto& entry : entries) {
        double s = sigmoid(entry.eval, k);
        derivative += (entry.result - s) * s * (1 - s) * entry.eval;
    }

    return derivative * (-2.0 / entries.size());
}

double computeTotalError(std::vector<Entry> &entries, double k) {
    double loss {0};
    for (auto & entry : entries) {
        loss += std::pow((entry.result - sigmoid(entry.eval, k)), 2);
    }
    return loss / entries.size();
}

void updateSingleEntryGradient(Entry &entry, double k, WeightVector& weights, GradientVector& gradients) {
    double eval = computeLinearEvaluation(entry, weights);
    double s = sigmoid(eval, k);
    double phaseCoefMG = entry.phase / 24;
    double phaseCoefEG = (24 - entry.phase) / 24;

    for (auto& coef : entry.coefs) {
        gradients[coef.index][mg] = (entry.result - s) * s * (1 - s) * phaseCoefMG * (coef.wCoef - coef.bCoef);
        gradients[coef.index][eg] = (entry.result - s) * s * (1 - s) * phaseCoefEG * (coef.wCoef - coef.bCoef);
    }
}

void computeBatchedGradient(std::vector<Entry>& entries, double k, WeightVector& weights, std::uint32_t batchIndex, GradientVector& gradients) {
    for (std::uint32_t index = batchIndex * batchSize; index < (batchIndex + 1) * batchSize; ++index) {
        Entry entry = entries[index];
        updateSingleEntryGradient(entry, k, weights, gradients);
    }
}

double computeLinearEvaluation(Entry &entry, WeightVector& weights) {
    double eval{};
    for (auto& coef : entry.coefs) {
        eval += (weights[coef.index][mg] * (coef.wCoef - coef.bCoef) * entry.phase + weights[coef.index][eg] * (coef.wCoef - coef.bCoef) * (24 - entry.phase)) / 24;
    }
    return eval;
}

double computeTotalError(std::vector<Entry> &entries, WeightVector& weights, double k) {
    double loss {0};
    for (auto & entry : entries) {
        double eval = computeLinearEvaluation(entry, weights);
        loss += std::pow((entry.result - sigmoid(eval, k)), 2);
    }
    return loss / entries.size();
}
