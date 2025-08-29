#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>
#include <algorithm>

#include "UDPListener.h"
#include "SocketUtils.h"
#include "Event.h"


std::mutex mtx;
bool running = true;
std::condition_variable cv;
int port;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    // TODO: Send a message to the listener to stop
    Exchange::SocketUtils::sendUDPMessage(port, "QUIT");
    // running = false;
}

void messageHandler(const std::string& message) {
    std::cout << "Received: " << message << std::endl;
    
    Exchange::EventType eventType = Exchange::toEventType(message);
    
    if (eventType == Exchange::EventType::Quit) {
        std::cout << "Received quit command. Shutting down..." << std::endl;
        {
          std::lock_guard<std::mutex> lock{mtx};
          running = false;
        }
        cv.notify_all();
    }
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
        Exchange::UDPListener listener(port);
    
      std::cout << "UDP Exchange Server running on port " << port << std::endl;
      std::cout << "Press Ctrl+C to stop..." << std::endl;

      auto handle = listener.subscribe(messageHandler);

      std::unique_lock<std::mutex> lock{mtx};
      cv.wait(lock, []{return !running;});
    }
    
    std::cout << "Shutting down..." << std::endl;
    std::cout << "Server stopped." << std::endl;
    
    return 0;
}

