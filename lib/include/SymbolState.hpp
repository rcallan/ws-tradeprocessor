#ifndef ____SymbolState__
#define ____SymbolState__

#include <deque>
#include <json/json.h>
#include <unordered_map>

struct SymbolState {
    std::deque<Json::Value> trades;
    std::unordered_map<std::string, double> features;
};

#endif