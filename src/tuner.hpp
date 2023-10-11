#pragma once

#include "evaluate.hpp"

#include <cstdint>
#include <vector>

constexpr std::uint32_t nTerms = sizeof(EvalVector) / sizeof(EvalTrace);
constexpr std::uint32_t nPhase = 2;
constexpr std::uint8_t mg = 0;
constexpr std::uint8_t eg = 0;

constexpr std::uint32_t nEpochs = 100;
constexpr double learningRate = 0.1;
constexpr std::uint32_t batchSize = 256;
constexpr  std::uint32_t kPrecisionEpochs = 250;
constexpr double learningRateK = 0.001;

using WeightVector = double[nTerms][nPhase];
using GradientVector = double[nTerms][nPhase];

struct Coef {
    std::uint32_t index;
    std::uint8_t wCoef;
    std::uint8_t bCoef;
};

struct Entry {
    double result;
    double phase;
    Score eval;
    std::vector<Coef> coefs;
};

void startTuner();
bool initEntries(std::vector<Entry>& entries);
double computeOptimalK(std::vector<Entry>& entries);
void computeBatchedGradient(std::vector<Entry>& entries, double k, WeightVector& weights, std::uint32_t batchIndex, GradientVector& gradients);
void updateSingleEntryGradient(Entry& entry, double k, WeightVector& weights, GradientVector& gradients);
double computeLinearEvaluation(Entry& entry, WeightVector& weights);
double computeTotalError(std::vector<Entry>& entries, WeightVector& weights, double k);
double computeTotalError(std::vector<Entry>& entries, double k);
double sigmoid(double x, double k);
double computeDerivativeOverK(std::vector<Entry>& entries, double k);