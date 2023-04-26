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
    void process(Json::Value& entry, std::unordered_map<std::string, std::unordered_map<std::string, double>>& calcInfoMap) {
        static_cast<T*>(this)->processEntry(entry, calcInfoMap);
    }

    std::string getMapKey() {
        return static_cast<T*>(this)->getCalcMapKey();
    }
};

class MaxTimeGapCalc : public Calc<MaxTimeGapCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, std::unordered_map<std::string, double>>& calcInfoMap) {
        long ts = entry["t"].asInt64();
        auto& temp = calcInfoMap[entry["s"].asString()];
        if (temp.count(mapKey) == 0) {
            temp[mapKey] = 0;
        } else if (ts - temp["maxGap.prevTime"] > temp[mapKey]) {
            temp[mapKey] = ts - temp["maxGap.prevTime"];
        }
        temp["maxGap.prevTime"] = ts;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"maxGap"};
};

class VolumeCalc : public Calc<VolumeCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, std::unordered_map<std::string, double>>& calcInfoMap) {
        double entryVol = entry["v"].asDouble();
        calcInfoMap[entry["s"].asString()][mapKey] += entryVol;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"vol"};
};

class MaxPriceCalc : public Calc<MaxPriceCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, std::unordered_map<std::string, double>>& calcInfoMap) {
        double entryPrice = entry["p"].asDouble();
        auto& temp = calcInfoMap[entry["s"].asString()][mapKey];
        if (entryPrice > temp) temp = entryPrice;
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"maxPrice"};
};

class WeightedAvgPriceCalc : public Calc<WeightedAvgPriceCalc> {
public:
    void processEntry(Json::Value& entry, std::unordered_map<std::string, std::unordered_map<std::string, double>>& calcInfoMap) {
        double vol = entry["v"].asDouble();
        double price = entry["p"].asDouble();
        // std::cout << "price and volume are " << price << " " << vol << std::endl;
        auto& temp = calcInfoMap[entry["s"].asString()];
        temp["wap.numer"] += vol * price;
        temp["wap.denom"] += vol;
        temp[mapKey] = temp["wap.numer"] / temp["wap.denom"];
    }

    std::string getCalcMapKey() { return mapKey; }

private:
    std::string mapKey {"weightedAvgPrice"};
};

#endif