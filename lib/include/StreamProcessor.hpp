#ifndef ____StreamProcessor__
#define ____StreamProcessor__

// #include "LineSplitter.h"
#include <vector>
#include <string>
#include <fstream>
#include "Calc.hpp"
#include <set>
#include <memory>
#include "SymbolState.hpp"

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
    StreamProcessor(auto& cim) : symbolStates(cim) { }
    StreamProcessor(auto& cim, auto cs) : calcs(cs), symbolStates(cim) { }
    StreamProcessor(std::string& rp, auto& cim) : readPath(rp), symbolStates(cim) { }
    StreamProcessor(std::string& rp, auto& cim, auto cs) : readPath(rp), calcs(cs), symbolStates(cim) { }

    void setCalcs(auto cs) {calcs = cs;}

    void process(moodycamel::BlockingConcurrentQueue<Json::Value>& q) {
        Json::Value item;
        
        while (true) {
            q.wait_dequeue(item);

            // std::cout << "processing new entry " << item["s"] << " " << item["t"] << " " << item["v"] << std::endl;

            const std::string& symbol = item["s"].asString();
            SymbolState& state = symbolStates[symbol];
            state.trades.push_back(item);

            tupleProcess(item, state);
        }
    }

    template <int n = std::tuple_size<T>::value>
    inline void tupleProcess(Json::Value& entry, SymbolState& state) {
        if constexpr (n > 1) {
            tupleProcess<n - 1>(entry, state);
        }
        
        std::get<n - 1>(calcs).process(entry, state);
    }

private:
    std::string readPath;
    T calcs {};

    std::unordered_map<std::string, SymbolState>& symbolStates;
    std::vector<std::string> mapKeys;
};

#endif