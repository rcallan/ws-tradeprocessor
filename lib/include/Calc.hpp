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
        for (auto it = s.trades.begin(); it != s.trades.end(); ++it) {
            static_cast<T*>(this)->processEntry(it, s);
        }
    }
};

class MaxTimeGapCalc : public Calc<MaxTimeGapCalc> {
public:
    void processEntry(std::deque<Json::Value>::iterator entry, SymbolState& s) {
        long ts = (*entry)["t"].asInt64();
        if (ts - s.maxGapState.prevTime > s.features.maxGap && s.maxGapState.prevTime > 0) {
            s.features.maxGap = ts - s.maxGapState.prevTime;
        }
        s.maxGapState.prevTime = ts;
    }
};

class VolumeCalc : public Calc<VolumeCalc> {
public:
    void processEntry(std::deque<Json::Value>::iterator entry, SymbolState& s) {
        double tradeVol = (*entry)["v"].asDouble();
        s.features.vol += tradeVol;
    }
};

class TradespersecCalc : public Calc<TradespersecCalc> {
public:
    void processEntry(std::deque<Json::Value>::iterator entry, SymbolState& s) {
        double tps;

        if (s.trades.size() == 0) return;

        double duration = s.trades.back()["t"].asInt64() - s.trades.front()["t"].asInt64();

        if (s.trades.size() == 1 || duration == 0.0) tps = 0.0;
        else tps = s.trades.size() / duration * 1000.0;

        s.features.tps = tps;
    }
};

class MaxPriceCalc : public Calc<MaxPriceCalc> {
public:
    void processEntry(std::deque<Json::Value>::iterator entry, SymbolState& s) {
        double entryPrice = (*entry)["p"].asDouble();
        auto& maxPrice = s.features.maxPrice;
        if (entryPrice > maxPrice) maxPrice = entryPrice;
    } 
};

class WeightedAvgPriceCalc : public Calc<WeightedAvgPriceCalc> {
public:
    void processEntry(std::deque<Json::Value>::iterator entry, SymbolState& s) {
        double vol = (*entry)["v"].asDouble();
        double price = (*entry)["p"].asDouble();
        s.wapState.numer += vol * price;
        s.wapState.denom += vol;
        s.features.weightedAvgPrice = s.wapState.numer / s.wapState.denom;
    }
};

// calculates upticks and downticks by looping through trades in order they were received
class TickImbalanceCalc : public Calc<TickImbalanceCalc> {
public:
    void processEntry(std::deque<Json::Value>::iterator entry, SymbolState& s) {
        if (entry == s.trades.begin()) return;

        double priceChange = (*entry)["p"].asDouble() - (*std::prev(entry))["p"].asDouble();

        if (priceChange > 0) {
            ++s.tiState.upticks;
        } else if (priceChange < 0) {
            ++s.tiState.downticks;
        }

        int total = s.tiState.upticks + s.tiState.downticks;

        s.features.tickImbalance = (total > 0) ? static_cast<double>(s.tiState.upticks - s.tiState.downticks) / total : 0.0;
    }
};

#endif