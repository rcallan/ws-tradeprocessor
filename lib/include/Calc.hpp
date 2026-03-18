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
#include "SymbolState.hpp"

#include <json/json.h>

using Clock = std::chrono::steady_clock;

template <typename T>
class Calc {
public:
    void process(Json::Value& entry, SymbolState& symbolState) {
        static_cast<T*>(this)->processEntry(entry, symbolState);
    }

    std::string getMapKey() {
        return static_cast<T*>(this)->getCalcMapKey();
    }
};

class MaxTimeGapCalc : public Calc<MaxTimeGapCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& symbolState) {
        long ts = entry["t"].asInt64();
        if (symbolState.features.count(mapKey) == 0) {
            symbolState.features[mapKey] = 0;
        } else if (ts - symbolState.features["maxGap.prevTime"] > symbolState.features[mapKey]) {
            symbolState.features[mapKey] = ts - symbolState.features["maxGap.prevTime"];
        }
        symbolState.features["maxGap.prevTime"] = ts;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"maxGap"};
};

class VolumeCalc : public Calc<VolumeCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& symbolState) {
        double vol = entry["v"].asDouble();
        symbolState.features[mapKey] += vol;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"vol"};
};

class TradespersecCalc : public Calc<TradespersecCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& symbolState) {
        long ts = entry["t"].asInt64();
        // if (ts < tsWindow[symbol].front() && tsWindow[symbol].size() > 1) {
        //     std::cout << "nonchrono event detected" << std::endl;
        // }
        auto now = Clock::now();
        auto cutoff = now - std::chrono::minutes(10);

        long cutoff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            cutoff.time_since_epoch()
        ).count();

        while (!symbolState.trades.empty() && symbolState.trades.front()["t"].asInt64() < cutoff_ms) {
            symbolState.trades.pop_front();
        }

        symbolState.features[mapKey] = tradespersec(symbolState);
    }

    double tradespersec(SymbolState& symbolState) {
        if (symbolState.trades.size() <= 1) return 0.0;

        double duration = symbolState.trades.back()["t"].asInt64() - symbolState.trades.front()["t"].asInt64();

        if (duration == 0.0) return symbolState.trades.size();
        return symbolState.trades.size() / duration * 1000.0;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"tps"};
};

class MaxPriceCalc : public Calc<MaxPriceCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& symbolState) {
        double entryPrice = entry["p"].asDouble();
        auto& maxPrice = symbolState.features[mapKey];
        if (entryPrice > maxPrice) maxPrice = entryPrice;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"maxPrice"};
};

class WeightedAvgPriceCalc : public Calc<WeightedAvgPriceCalc> {
public:
    void processEntry(Json::Value& entry, SymbolState& symbolState) {
        double vol = entry["v"].asDouble();
        double price = entry["p"].asDouble();
        symbolState.features["wap.numer"] += vol * price;
        symbolState.features["wap.denom"] += vol;
        symbolState.features[mapKey] = symbolState.features["wap.numer"] / symbolState.features["wap.denom"];
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"weightedAvgPrice"};
};

#endif