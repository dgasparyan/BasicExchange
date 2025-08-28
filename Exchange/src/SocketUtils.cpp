#include "SocketUtils.h"

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

namespace SocketUtils {

  class SockFDGuard {
    int fd_;
    public:
    SockFDGuard(int fd) : fd_(fd) {}
    ~SockFDGuard() {
        close(fd_);
    }
  };
  
  bool sendUDPMessage(int port, const std::string& message) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "socket creation failed" << std::endl;
        return false;
    }
    SockFDGuard guard(sockfd);

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port); // target port
    inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr); // target IP

    const char* msg = message.c_str();

    ssize_t sent = sendto(sockfd, msg, strlen(msg), 0,
                          reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
    if (sent < 0) {
        std::cerr << "sendto failed" << std::endl;
        return false;
    }

    return true;
  }
} 