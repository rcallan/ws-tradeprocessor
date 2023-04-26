#include <boost/program_options.hpp>
#include <boost/asio/ssl/context.hpp>
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <memory>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "WSSession.hpp"

namespace po = boost::program_options;

WSSession::WSSession(std::string fileName, std::condition_variable& cv_, std::shared_ptr<moodycamel::BlockingConcurrentQueue<Json::Value>> q_) : cv(cv_), q(q_) {
    // get logger here

    po::options_description config("Configuration");
    config.add_options()
        ("API_Token", po::value<std::string>()->default_value(""), "API Key assigned by Finnhub");
    config.add_options()
        ("Symbol_List", po::value<std::vector<std::string> >()->multitoken(), "List of Sysmbols to be subscribed");
    
    po::variables_map vm;

    std::ifstream ifs(fileName);
    if (ifs) {
        std::cout << "parsing config file" << std::endl;
        boost::program_options::store(boost::program_options::parse_config_file(ifs, config), vm);
        boost::program_options::notify(vm);
    }

    token = vm["API_Token"].as<std::string>();

    for (auto& symbol : vm["Symbol_List"].as<std::vector<std::string>>()) {
        // logger_->info("Subscribing to the symbol [{}]", symbol);
        subscriptions.push_back("{\"type\":\"subscribe\",\"symbol\":\"" + symbol +"\"}");
    }

}

typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

void WSSession::on_message(websocketpp::connection_hdl, websocketpp::client<websocketpp::config::asio_tls_client>::message_ptr msg) {
	// std::cout << msg->get_payload() << std::endl;

    Json::Reader reader;
    Json::Value completeJsonData;
    std::stringstream ss;

    curlpp::Cleanup cleaner;

    // could possibly move this into a processing thread and just place raw payloads into the queue
    reader.parse(msg->get_payload(), completeJsonData, false);

    for (unsigned i = 0; i < completeJsonData["data"].size(); ++i) {
        // std::cout << completeJsonData["data"][i] << std::endl;
        q->enqueue(completeJsonData["data"][i]);
    }
    std::cout << "approximate size of the queue so far is " << q->size_approx() << std::endl;
}

void WSSession::on_open(websocketpp::connection_hdl hdl) {
    // would like to use a condition variable here to notify other thread that connection is ready
    std::cout << "connection has been opened" << std::endl;
    cv.notify_one();
}

static context_ptr on_tls_init() {
    // establishes a SSL connection
    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception &e) {
        std::cout << "Error in context pointer: " << e.what() << std::endl;
    }
    return ctx;
}

void WSSession::connect() {
    try {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        c.init_asio();
        c.set_tls_init_handler(bind(&on_tls_init));

        // Register our message handler
        c.set_message_handler(bind(&WSSession::on_message, this, ::_1, ::_2));
        
        // register our open handler
        c.set_open_handler(bind(&WSSession::on_open, this, ::_1));

        websocketpp::lib::error_code ec;
        websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr con = c.get_connection("wss://ws.finnhub.io/?token=" + token, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }

        // gets a connection handle so we can send msgs to the server elsewhere
        hdl = con->get_handle();

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);        

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << "error encountered within the subscribe function" << std::endl;
        std::cout << e.what() << std::endl;
    }

}

void WSSession::subscribe() {
    for (const auto& s : subscriptions) {
        websocketpp::lib::error_code ec;

        std::cout << "subscribing to " << s << std::endl;
        // con->send(s, websocketpp::frame::opcode::text);
        c.send(hdl, s, websocketpp::frame::opcode::text, ec);

        // The most likely error that we will get is that the connection is
        // not in the right state. Usually this means we tried to send a
        // message to a connection that was closed or in the process of
        // closing
        if (ec) {
            c.get_alog().write(websocketpp::log::alevel::app,
                "Send Error: " + ec.message());
        }
    }
}

void WSSession::runLoop() {
    auto nextUpdateTime = 0L;
    while (true) {
        if (time(NULL) < nextUpdateTime) {
            continue;
        }
        nextUpdateTime = time(NULL) + 5;
    }
}