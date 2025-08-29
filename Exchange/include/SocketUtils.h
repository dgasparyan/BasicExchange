#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Exchange {

namespace SocketUtils {
    bool sendUDPMessage(int port, const std::string& message);
}

} // namespace Exchange

#endif // SOCKET_UTILS_H 