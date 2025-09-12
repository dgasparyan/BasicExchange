#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <thread>

#include "SocketUtils.h"
#include "Event.h"
#include "Exchange.h"
#include "UDPListener.h"

#include "OrderBook.h"

int port;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    // TODO: Send a message to the listener to stop
    Exchange::SocketUtils::sendUDPMessage(port, "QUIT");
    // running = false;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <port>" << std::endl;
    std::cout << "  port: UDP port to listen on (e.g., 8080)" << std::endl;
}

int parsePort(const char* portStr) {
    try {
        int port = std::stoi(portStr);
        if (port <= 0 || port > 65535) {
            throw std::out_of_range("Port out of range");
        }
        return port;
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid port number: " + std::string(portStr));
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        port = parsePort(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    {
      Exchange::CsvEventParser eventParser;
      Exchange::UDPListener listener(port);

      // TODO: this whole creation needs to be fixed, should be using one report sink per book to reduce contention
      Exchange::ReportSink reportSink;
      Exchange::OrderBookManager::OrderBookMap orderBookMap;
      for (std::string_view symbol : {"AAPL", "GOOGL", "MSFT", "AMZN", "META", "NVDA"}) {
        auto sink = std::make_unique<Exchange::ReportSink>();
        orderBookMap.emplace(symbol, std::make_unique<Exchange::OrderBook<Exchange::ReportSink>>(Exchange::Symbol{symbol}, std::move(sink)));
      }
      // const auto numThreads = std  ::max(static_cast<int>(std::thread::hardware_concurrency() / 2), 2);
      const auto numThreads = 3;
      
      Exchange::OrderBookManager orderBookManager {std::move(orderBookMap), numThreads};
      Exchange::Exchange  exchange(listener, eventParser, orderBookManager);
    
      std::cout << "UDP Exchange Server running on port " << port << std::endl;
      std::cout << "Press Ctrl+C to stop..." << std::endl;

      exchange.start();
    }
    
    std::cout << "Shutting down..." << std::endl;
    std::cout << "Server stopped." << std::endl;
    
    return 0;
}

