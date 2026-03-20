#ifndef ____SymbolState__
#define ____SymbolState__

#include <deque>
#include <json/json.h>
#include <unordered_map>
#include <mutex>
#include "Features.hpp"

struct MaxGapState {
    long prevTime = 0;
};

struct WAPState {
    double numer = 0.0;
    double denom = 0.0;
};

struct SymbolState {
    std::mutex m;
    std::deque<Json::Value> trades;
    Features features;

    MaxGapState maxGapState;
    WAPState wapState;

    void resetFeatures() {
        features.maxGap = 0.0;
        features.maxPrice = 0.0;
        features.tps = 0.0;
        features.vol = 0.0;
        features.weightedAvgPrice = 0.0;

        maxGapState.prevTime = 0;
        wapState.numer = 0.0;
        wapState.denom = 0.0;
    }
};

#endif