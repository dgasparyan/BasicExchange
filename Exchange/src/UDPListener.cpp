#include "UDPListener.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "SocketUtils.h"
#include "EventParser.h"

namespace Exchange {

namespace {
  int getNextHandle() {
    // TODO: This should be randomly generated
    static int handle = 0;
    return handle++;
  }
}

UdpSubscriptionHandle::UdpSubscriptionHandle(UDPListener* listener, int handle) : listener_(listener), handle_(handle) {}

UdpSubscriptionHandle::~UdpSubscriptionHandle() {
  if (listener_ && handle_ >= 0) {
    listener_->unsubscribe(handle_);
  }
}

UDPListener::UDPListener(int port) : socketFd_(-1), port_(port) {
    bindToPort(port_);
    startListening();
}

UDPListener::~UDPListener() {
    stopListening();
    if (socketFd_ >= 0) {
        close(socketFd_);
    }
}

std::unique_ptr<SubscriptionHandle> UDPListener::subscribe(MessageCallback callback) {
  std::lock_guard<std::mutex> lock(cbMutex_);
  int handle = getNextHandle();
  callbacks_[handle] = callback;
  return std::make_unique<UdpSubscriptionHandle>(this, handle);
}

bool UDPListener::unsubscribe(int handle) {
  std::lock_guard<std::mutex> lock(cbMutex_);
  return callbacks_.erase(handle) > 0;
}

void UDPListener::bindToPort(int port) {
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

bool UDPListener::startListening() {
    listenerThread_ = std::thread(&UDPListener::listenLoop, this);
    return true;
}

void UDPListener::stopListening() {
    if (listenerThread_.joinable()) {
        // TODO: Better to use an "internal stop message" so it doesn't affect the subscribers
        SocketUtils::sendUDPMessage(port_, "QUIT");
        listenerThread_.join();
    }
    std::cout << "Stopped listening for UDP messages." << std::endl;
}

void UDPListener::listenLoop() {
    char buffer[4096];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    std::cout << "Started listening for UDP messages..." << std::endl;
    while (true) {
        // Clear client address structure
        memset(&clientAddr, 0, sizeof(clientAddr));
        
        // Receive message (blocking)
        ssize_t bytesReceived = recvfrom(socketFd_, buffer, sizeof(buffer) - 1, 0,
                                        (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (bytesReceived > 0) {
            // Null-terminate the received data
            buffer[bytesReceived] = '\0';
            
            std::cout << "DEBUG: Received " << bytesReceived << " bytes" << std::endl;
            std::string message{buffer};

            // TODO: yes, it's not great calling user-supplied code under our lock. 
            // either copy/snapshot or do atomic<Umap*> and swap on update
            {
              std::lock_guard<std::mutex> lock(cbMutex_);
              for (const auto& [_, callback] : callbacks_) {
                  callback(message);
              }
            }

            if (toEventType(std::string(buffer)) == EventType::Quit) {
              break;
            }

        } else if (bytesReceived < 0) {
            // Error occurred
            std::cerr << "Error receiving UDP message: " << strerror(errno) << std::endl;
        }
    }
}

} // namespace Exchange