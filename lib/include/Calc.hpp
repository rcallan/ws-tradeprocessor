#ifndef ____Calc__
#define ____Calc__

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <fstream>

#include <json/json.h>

template <typename T>
class Calc {
public:
    void process(Json::Value& entry, std::unordered_map<std::string, double>& symbolState) {
        static_cast<T*>(this)->processEntry(entry, symbolState);
    }

    std::string getMapKey() {
        return static_cast<T*>(this)->getCalcMapKey();
    }
};

class MaxTimeGapCalc : public Calc<MaxTimeGapCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, double>& symbolState) {
        long ts = entry["t"].asInt64();
        if (symbolState.count(mapKey) == 0) {
            symbolState[mapKey] = 0;
        } else if (ts - symbolState["maxGap.prevTime"] > symbolState[mapKey]) {
            symbolState[mapKey] = ts - symbolState["maxGap.prevTime"];
        }
        symbolState["maxGap.prevTime"] = ts;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"maxGap"};
};

class VolumeCalc : public Calc<VolumeCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, double>& symbolState) {
        double vol = entry["v"].asDouble();
        symbolState[mapKey] += vol;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"vol"};
};

class MaxPriceCalc : public Calc<MaxPriceCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, double>& symbolState) {
        double entryPrice = entry["p"].asDouble();
        auto& maxPrice = symbolState[mapKey];
        if (entryPrice > maxPrice) maxPrice = entryPrice;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"maxPrice"};
};

class WeightedAvgPriceCalc : public Calc<WeightedAvgPriceCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, double>& symbolState) {
        double vol = entry["v"].asDouble();
        double price = entry["p"].asDouble();
        symbolState["wap.numer"] += vol * price;
        symbolState["wap.denom"] += vol;
        symbolState[mapKey] = symbolState["wap.numer"] / symbolState["wap.denom"];
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"weightedAvgPrice"};
};

#endif