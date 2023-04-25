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

#include "WSSession.hpp"

void process(std::shared_ptr<moodycamel::BlockingConcurrentQueue<Json::Value>> q) {
    Json::Value item;
    
    while (true) {
        q->wait_dequeue(item);

        std::cout << "symbol" << item["s"] << " price " << item["p"] << " vol " << item["v"] << std::endl;
    }
}

int main(int argc, const char* argv[]) {
    unsigned threadCount = std::thread::hardware_concurrency();

    threadCount = (threadCount > 0) ? threadCount : 1;

    std::cout << "using dp::thread-pool version " << THREADPOOL_VERSION << " with " << threadCount << " threads" << std::endl;

    // created a thread pool with n - 1 threads since one of the available threads is being used for main
    dp::thread_pool pool(threadCount - 1);

    std::mutex mtx;
    std::condition_variable condVar;

    auto q = std::make_shared<moodycamel::BlockingConcurrentQueue<Json::Value>>();
    std::cout << "is the queue lock free on this platform " << q->is_lock_free() << std::endl;

    auto sess = std::make_shared<WSSession>(argv[1], condVar, q);
    // forms a ws connection with the finhubb server and starts the ASIO io_service run loop
    pool.enqueue_detach(&WSSession::connect, sess);

    // send subscription messages once a connection is opened
    std::unique_lock<std::mutex> lck(mtx);
    condVar.wait(lck);
    sess->subscribe();

    // process items which have been submitted to the queue
    // could spawn more processing threads if the queue needs to be processed more quickly
    pool.enqueue_detach(process, q);

    return 0;
}