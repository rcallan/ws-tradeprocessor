#ifndef ____Scheduler__
#define ____Scheduler__

#include <atomic>
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
        worker = std::thread(&Scheduler::run, this);
    }

    void stop() {
        running.store(false);
        cv.notify_one();
        if (worker.joinable()) {
            worker.join();
        }
    }

private:
    std::atomic<bool> running{true};
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    Processor& sp;

    void run() {
        using namespace std::chrono;

        // auto interval = minutes(1);
        auto interval = seconds(10);

        // align to next boundary (steady clock)
        auto next_tick = steady_clock::now() + interval;

        std::unique_lock<std::mutex> lock(mtx);

        while (running.load()) {
            // wait until next_tick OR until notified (e.g. shutdown)
            if (cv.wait_until(lock, next_tick, [this]() {
                    return !running.load();
                })) {
                // woke due to stop signal
                break;
            }

            // timeout → scheduled tick reached
            if (!running.load()) break;

            lock.unlock();

            sp.computeFeatures();

            lock.lock();

            // schedule next tick
            next_tick += interval;
        }
    }

};

#endif