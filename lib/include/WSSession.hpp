#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/transport/asio/security/tls.hpp>

#include <concurrentqueue/blockingconcurrentqueue.h>

#include <condition_variable>
#include <mutex>

class WSSession {
public:
    WSSession(std::string cfgName, std::condition_variable& cv_, moodycamel::BlockingConcurrentQueue<Json::Value>& q_);

    void connect();
    void subscribe();
    void runLoop();
    void on_message(websocketpp::connection_hdl, websocketpp::client<websocketpp::config::asio_tls_client>::message_ptr msg);
    void on_open(websocketpp::connection_hdl hdl);
private:
    websocketpp::client<websocketpp::config::asio_tls_client> c;
    websocketpp::connection_hdl hdl;
    std::string token;
    std::vector<std::string> subscriptions;
    std::condition_variable& cv;
    moodycamel::BlockingConcurrentQueue<Json::Value>& q;
};