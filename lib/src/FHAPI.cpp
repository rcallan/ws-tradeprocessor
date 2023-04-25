#include <string>
#include <vector>
#include <json/json.h>
#include <iostream>
#include <unordered_map>
#include <cmath>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include <concurrentqueue/blockingconcurrentqueue.h>

#include <thread_pool/thread_pool.h>

void queryPrices(const std::vector<std::string>& symbols, moodycamel::BlockingConcurrentQueue<Json::Value>& q) {
    std::string url;
    Json::Reader reader;
    // this will contain complete JSON data
    Json::Value completeJsonData;
    std::stringstream ss;

    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    while (true) {
        for (unsigned i = 0; i < symbols.size(); ++i) {
            try {
                // url = std::format("https://finnhub.io/api/v1/quote?symbol={}&token=ch0met9r01qhadkoem2gch0met9r01qhadkoem30", symbols[i]);
                url = "https://finnhub.io/api/v1/quote?symbol=" + symbols[i] + "&token=ch0met9r01qhadkoem2gch0met9r01qhadkoem30";

                // setting the url
                request.setOpt(new curlpp::options::Url(url));

                // the overloaded << operator performs the request before printing to standard out
                // std::cout << request << std::endl;
                ss << request;

                // std::cout << curlpp::options::Url(url) << std::endl;
                
                // reader reads the data and stores it in completeJsonData
                reader.parse(ss.str(), completeJsonData, false);

                std::stringstream temp;
                ss.swap(temp);

                // auto tt = std::chrono::system_clock::to_time_t(std::chrono::sys_seconds{std::chrono::seconds(completeJsonData["t"].asInt())});

                // complete JSON data
                // std::cout << "Complete JSON data: " << std::endl << completeJsonData << std::endl;
                // std::cout << "symbol: " << symbols[i] << ", " << "current price: " << std::round(completeJsonData["c"].asDouble() * 100.0) / 100.0 
                //     << ", " << "timestamp: " << std::put_time(std::localtime(&tt), "%F %T.\n") << std::flush;
                
                completeJsonData["sym"] = symbols[i];

                q.enqueue(completeJsonData);
            } catch ( curlpp::LogicError & e ) {
                std::cout << e.what() << std::endl;
            } catch ( curlpp::RuntimeError & e ) {
                std::cout << e.what() << std::endl;
            }
        }
        // std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    
}

void processItem(Json::Value& item, std::unordered_map<std::string, double>& cim, std::mutex& mtx) {
    // auto tt = std::chrono::system_clock::to_time_t(std::chrono::sys_seconds{std::chrono::seconds(item["t"].asInt())});

    // std::cout << "symbol: " << item["sym"] << ", " << "current price: " << std::round(item["c"].asDouble() * 100.0) / 100.0 
    //     << ", " << "timestamp: " << std::put_time(std::localtime(&tt), "%F %T.\n") << std::flush;

    // if (item["sym"] == "V")
    //     std::cout << std::endl;
    std::unique_lock<std::mutex> ul(mtx);
    cim[std::string(item["sym"].asCString())] = std::round(item["c"].asDouble() * 100.0) / 100.0;
    std::cout << "updated the price of " << item["sym"] << " in calcInfoMap" << std::endl;
}

void useApi() {
    std::vector<std::string> symbols {"AMZN", "GOOG", "BBY", "BLK", "C", "JPM", "NVDA", "SEDG", "VZ", "V"};

    auto threadCount = std::thread::hardware_concurrency();

    threadCount = (threadCount > 0) ? threadCount : 1;

    dp::thread_pool pool(threadCount);
    moodycamel::BlockingConcurrentQueue<Json::Value> q;

    pool.enqueue_detach(queryPrices, std::ref(symbols), std::ref(q));

    Json::Value item;

    std::unordered_map<std::string, double> calcInfoMap;
    std::mutex mtx;

    while (true) {
        q.wait_dequeue(item);

        pool.enqueue_detach(processItem, std::ref(item), std::ref(calcInfoMap), std::ref(mtx));
    }
}