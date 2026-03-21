#ifndef ____Scheduler__
#define ____Scheduler__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "StreamProcessor.hpp"

#include "spdlog/spdlog.h"

template<typename Processor>
class Scheduler {
public:
    Scheduler() = default;
    Scheduler(auto& _sp) : sp(_sp) { }

    void start() {
        worker = std::jthread(&Scheduler::run, this);
    }

    void stop() {
        worker.request_stop();
        cv.notify_one();  // wake if waiting
    }

private:
    std::mutex mtx;
    std::condition_variable_any cv;  // must be condition_variable_any
    std::jthread worker;
    Processor& sp;

    void run(std::stop_token st) {
        using namespace std::chrono;

        auto interval = seconds(10);
        auto next_tick = steady_clock::now() + interval;

        std::unique_lock<std::mutex> lock(mtx);

        while (!st.stop_requested()) {

            if (cv.wait_until(lock, st, next_tick, [&st]() {
                    return st.stop_requested();
                })) {
                break;
            }

            lock.unlock();

            sp.computeFeatures();

            lock.lock();

            next_tick += interval;
        }
    }
};

#endif