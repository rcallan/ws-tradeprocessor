#include <iostream>
#include <unordered_map>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <thread_pool/thread_pool.h>
#include <thread_pool/version.h>

#include <concurrentqueue/blockingconcurrentqueue.h>
#include <json/json.h>

#include "spdlog/spdlog.h"

#include "WSSession.hpp"
#include "Calc.hpp"
#include "StreamProcessor.hpp"

int main(int argc, const char* argv[]) {
    // auto logger = spdlog::basic_logger_mt("daily_logger", "logs/daily-log.txt");
    // logger->info("===== starting application =====");
    unsigned threadCount = std::thread::hardware_concurrency();

    threadCount = (threadCount > 0) ? threadCount : 1;

    spdlog::info("using dp::thread-pool version {} with {} threads", THREADPOOL_VERSION, threadCount);

    // created a thread pool with n - 1 threads since one of the available threads is being used for main
    dp::thread_pool pool(threadCount - 1);

    std::mutex mtx;
    std::condition_variable condVar;

    moodycamel::BlockingConcurrentQueue<Json::Value> q;
    spdlog::info("is the queue lock free on this platform {}", q.is_lock_free());

    WSSession sess(argv[1], condVar, q);
    // forms a ws connection with the finhubb server and starts the ASIO io_service run loop
    pool.enqueue_detach(&WSSession::connect, std::ref(sess));

    // we wait until we are subscribed before continuing
    std::unique_lock<std::mutex> lck(mtx);
    condVar.wait(lck);

    typedef std::tuple<MaxTimeGapCalc, VolumeCalc, WeightedAvgPriceCalc, MaxPriceCalc> CalcTypes;

    std::unordered_map<std::string, std::unordered_map<std::string, double>> calcInfoMap;

    StreamProcessor<CalcTypes> sp(calcInfoMap);

    // process items which have been submitted to the queue
    // could spawn more processing threads if the queue needs to be processed more quickly
    pool.enqueue_detach(&StreamProcessor<CalcTypes>::process, std::ref(sp), std::ref(q));

    auto keys = sp.getMapKeys();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        spdlog::info("approximate size of the queue so far is {}", q.size_approx());
        for (auto& [k, v] : calcInfoMap) {
            spdlog::info("{} - maxGap: {:.2f} ms, vol: {:.2f}, weightedAvgPrice: {:.2f}, maxPrice {:.2f}", k, 
                v["maxGap"], v["vol"], v["weightedAvgPrice"], v["maxPrice"]);
        }
        std::cout << std::endl;
    }

    return 0;
}