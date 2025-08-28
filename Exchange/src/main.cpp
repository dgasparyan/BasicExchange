#include <iostream>
#include <string>
#include <csignal>
#include <chrono>
#include <thread>
#include <algorithm>

#include "UDPListener.h"
#include "SocketUtils.h"


std::mutex mtx;
bool running = true;
std::condition_variable cv;
int port;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    // TODO: Send a message to the listener to stop
    SocketUtils::sendUDPMessage(port, "QUIT");
    // running = false;
}

void messageHandler(const std::string& message, const std::string& senderIP) {
    std::cout << "Received from " << senderIP << ": " << message << std::endl;
    
    std::string trimmedMessage = message;
    trimmedMessage.erase(0, trimmedMessage.find_first_not_of(" \t\r\n"));
    trimmedMessage.erase(trimmedMessage.find_last_not_of(" \t\r\n") + 1);
    
    std::string upperMessage = trimmedMessage;
    std::transform(upperMessage.begin(), upperMessage.end(), upperMessage.begin(), ::toupper);
    
    std::cout << "Trimmed message is [" << upperMessage << "]" << std::endl;
    
    if (upperMessage == "Q" || upperMessage == "QUIT") {
        std::cout << "Received quit command from " << senderIP << ". Shutting down..." << std::endl;
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

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Parse port number
    try {
        port = std::stoi(argv[1]);
        if (port <= 0 || port > 65535) {
            throw std::out_of_range("Port out of range");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid port number '" << argv[1] << "'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Create UDP listener
    UDPListener listener(port);
    
    // Start listening
    if (!listener.startListening(messageHandler)) {
        std::cerr << "Failed to start listening: " << listener.getLastError() << std::endl;
        return 1;
    }
    
    std::cout << "UDP Exchange Server running on port " << port << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    {
        std::unique_lock<std::mutex> lock{mtx};
        cv.wait(lock, []{return !running;});
    }
    std::cout << "Shutting down..." << std::endl;
    std::cout << "Server stopped." << std::endl;

    
    // Cleanup
    listener.stopListening();
    std::cout << "Server stopped." << std::endl;
    
    return 0;
}

