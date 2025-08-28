#include "UDPListener.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

UDPListener::UDPListener(int port) : socketFd_(-1), running_(false) {
    // Create UDP socket
    socketFd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd_ < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(socketFd_);
        socketFd_ = -1;
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }
    
    // Set socket to non-blocking mode
    int flags = fcntl(socketFd_, F_GETFL, 0);
    if (flags < 0) {
        close(socketFd_);
        socketFd_ = -1;
        throw std::runtime_error("Failed to get socket flags: " + std::string(strerror(errno)));
    }
    
    if (fcntl(socketFd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(socketFd_);
        socketFd_ = -1;
        throw std::runtime_error("Failed to set socket to non-blocking: " + std::string(strerror(errno)));
    }
    
    // Bind to port
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(socketFd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(socketFd_);
        socketFd_ = -1;
        throw std::runtime_error("Failed to bind to port " + std::to_string(port) + ": " + std::string(strerror(errno)));
    }
    
    std::cout << "UDP Listener initialized on port " << port << std::endl;
    std::cout << "Socket FD: " << socketFd_ << std::endl;
}

UDPListener::~UDPListener() {
    stopListening();
    if (socketFd_ >= 0) {
        close(socketFd_);
    }
}

bool UDPListener::startListening(MessageCallback callback) {
    if (running_) {
        lastError_ = "Already listening.";
        return false;
    }
    
    running_ = true;
    listenerThread_ = std::thread(&UDPListener::listenLoop, this, callback);
    
    std::cout << "Started listening for UDP messages..." << std::endl;
    std::cout << "Thread started, waiting for messages..." << std::endl;
    return true;
}

void UDPListener::stopListening() {
    if (running_) {
        running_ = false;
        if (listenerThread_.joinable()) {
            listenerThread_.join();
        }
        std::cout << "Stopped listening for UDP messages." << std::endl;
    }
}

void UDPListener::listenLoop(MessageCallback callback) {
    char buffer[4096];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    while (running_) {
        // Clear client address structure
        memset(&clientAddr, 0, sizeof(clientAddr));
        
        // Receive message (non-blocking)
        ssize_t bytesReceived = recvfrom(socketFd_, buffer, sizeof(buffer) - 1, 0,
                                        (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (bytesReceived > 0) {
            // Null-terminate the received data
            buffer[bytesReceived] = '\0';
            
            // Get sender IP address
            char senderIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, senderIP, INET_ADDRSTRLEN);
            
            std::cout << "DEBUG: Received " << bytesReceived << " bytes from " << senderIP << std::endl;
            
            callback(std::string(buffer), std::string(senderIP));
        } else if (bytesReceived < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available (non-blocking socket)
                // Sleep briefly to avoid busy-waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } else {
                // Actual error occurred
                std::cerr << "Error receiving UDP message: " << strerror(errno) << std::endl;
            }
        }
    }
} 