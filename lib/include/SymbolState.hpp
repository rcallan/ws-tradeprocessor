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
};

#endif