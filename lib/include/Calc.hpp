#ifndef ____Calc__
#define ____Calc__

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <fstream>
#include <deque>
#include <chrono>
#include <mutex>
#include "SymbolState.hpp"

#include <json/json.h>

using Clock = std::chrono::steady_clock;

template <typename T>
class Calc {
public:
    void process(SymbolState& s) {
        for (auto& entry : s.trades) {
            static_cast<T*>(this)->processEntry(entry, s);
        }
    }
};

class MaxTimeGapCalc : public Calc<MaxTimeGapCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& s) {
        long ts = entry["t"].asInt64();
        if (ts - s.maxGapState.prevTime > s.features.maxGap && s.maxGapState.prevTime > 0) {
            s.features.maxGap = ts - s.maxGapState.prevTime;
        }
        s.maxGapState.prevTime = ts;
    }

private:

};

class VolumeCalc : public Calc<VolumeCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& s) {
        double tradeVol = entry["v"].asDouble();
        s.features.vol += tradeVol;
    }

private:

};

class TradespersecCalc : public Calc<TradespersecCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& s) {
        double tps;

        if (s.trades.size() == 0) return;

        double duration = s.trades.back()["t"].asInt64() - s.trades.front()["t"].asInt64();

        if (s.trades.size() == 1 || duration == 0.0) tps = 0.0;
        else tps = s.trades.size() / duration * 1000.0;

        s.features.tps = tps;
    }

private:

};

class MaxPriceCalc : public Calc<MaxPriceCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& s) {
        double entryPrice = entry["p"].asDouble();
        auto& maxPrice = s.features.maxPrice;
        if (entryPrice > maxPrice) maxPrice = entryPrice;
    }

private:
    
};

class WeightedAvgPriceCalc : public Calc<WeightedAvgPriceCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& s) {
        double vol = entry["v"].asDouble();
        double price = entry["p"].asDouble();
        s.wapState.numer += vol * price;
        s.wapState.denom += vol;
        s.features.weightedAvgPrice = s.wapState.numer / s.wapState.denom;
    }

private:

};

#endif