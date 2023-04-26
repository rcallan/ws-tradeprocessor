#ifndef ____StreamProcessor__
#define ____StreamProcessor__

// #include "LineSplitter.h"
#include <vector>
#include <string>
#include <fstream>
#include "Calc.hpp"
#include <set>
#include <memory>

#include <json/json.h>

template<class T, std::size_t N>
concept has_tuple_element =
  requires(T t) {
    typename std::tuple_element_t<N, std::remove_const_t<T>>;
    { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
  };

template<class T>
concept tuple_like = !std::is_reference_v<T>
  && requires(T t) { 
    typename std::tuple_size<T>::type; 
    requires std::derived_from<
        std::tuple_size<T>, 
        std::integral_constant<std::size_t, std::tuple_size_v<T>>
    >;
  } && []<std::size_t... N>(std::index_sequence<N...>) { 
    return (has_tuple_element<T, N> && ...);
  }(std::make_index_sequence<std::tuple_size_v<T>>());

// restricting template parameter types to tuple like types
// found these particular concepts from https://stackoverflow.com/questions/68443804/c20-concept-to-check-tuple-like-types

template<tuple_like T>
class StreamProcessor {
public:
    StreamProcessor() = delete;
    StreamProcessor(auto& cim) : calcInfoMap(cim) { }
    StreamProcessor(auto& cim, auto cs) : calcs(cs), calcInfoMap(cim) { }
    StreamProcessor(std::string& rp, auto& cim) : readPath(rp), calcInfoMap(cim) { }
    StreamProcessor(std::string& rp, auto& cim, auto cs) : readPath(rp), calcs(cs), calcInfoMap(cim) { }

    void setCalcs(auto cs) {calcs = cs;}

    template <int n = std::tuple_size<T>::value>
    inline void tupleProcess(Json::Value& entry) {
        if constexpr (n > 1) {
            tupleProcess<n - 1>(entry);
        }
        std::get<n - 1>(calcs).process(entry, calcInfoMap);
    }

    template <int n = std::tuple_size<T>::value>
    inline void tupleGetMapKeys() {
        if constexpr (n > 1) {
            tupleGetMapKeys<n - 1>();
        }
        mapKeys.emplace_back(std::get<n - 1>(calcs).getMapKey());
    }

    std::vector<std::string> getMapKeys() {
        tupleGetMapKeys();
        return mapKeys;
    }

    long getCalcInfo(std::string&& symbol, std::string&& key) const {
        return calcInfoMap[symbol][key];
    }

private:
    std::string readPath;
    T calcs {};

    std::unordered_map<std::string, std::unordered_map<std::string, double>>& calcInfoMap;
    std::vector<std::string> mapKeys;
};

#endif